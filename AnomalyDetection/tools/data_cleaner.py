#!/usr/bin/env python3
"""Filter noise out of CSV telemetry dumps and keep the useful data."""

from __future__ import annotations

import argparse
import math
from pathlib import Path
import re
from typing import List, Optional, Sequence, Tuple


EXPECTED_HEADER = "time_ms,accel_mag_g,fan_pwm,vibration_pwm,current_a,event_code"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Remove boot logs or binary junk from a CSV file by keeping only rows "
            "that contain numeric comma-separated values."
        )
    )
    parser.add_argument(
        "-i",
        "--input",
        action="append",
        type=Path,
        help=(
            "CSV file or directory to clean. Provide multiple times to cover "
            "several locations. Defaults to the current working directory."
        ),
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help=(
            "Where to write the cleaned CSV. Defaults to "
            "<input stem>_clean<input suffix>."
        ),
    )
    parser.add_argument(
        "-c",
        "--expected-columns",
        type=int,
        default=None,
        help="Expected number of columns. If omitted, the first valid row decides.",
    )
    parser.add_argument(
        "--stop-marker",
        default="NORMAL data collection complete|DATA COLLECTION COMPLETE",
        help=(
            "Stop reading a file once any of these case-insensitive markers are seen. "
            "Separate multiple markers with '|'. Pass an empty string to disable "
            "(default: %(default)s)."
        ),
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report how many rows would be kept without writing a file.",
    )
    return parser.parse_args()


def _is_valid_number(token: str) -> bool:
    if not token:
        return False
    try:
        value = float(token)
    except ValueError:
        return False
    return math.isfinite(value)


def _clean_numeric_row(
    text: str, expected_columns: Optional[int]
) -> Tuple[Optional[str], Optional[int]]:
    stripped = text.strip()
    if not stripped:
        return None, expected_columns
    stripped = stripped.lstrip("\ufeff")
    if stripped.startswith("#"):
        return None, expected_columns
    parts = [piece.strip() for piece in stripped.split(",")]
    if expected_columns is not None and len(parts) != expected_columns:
        return None, expected_columns
    if not all(_is_valid_number(item) for item in parts):
        return None, expected_columns
    if expected_columns is None:
        expected_columns = len(parts)
    return ",".join(parts), expected_columns


ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")


def _strip_ansi(text: str) -> str:
    return ANSI_ESCAPE_RE.sub("", text)


def _should_stop(line: str, stop_markers: Sequence[str]) -> bool:
    if not stop_markers:
        return False
    sanitized = _strip_ansi(line).lower()
    return any(marker in sanitized for marker in stop_markers)


def clean_file(
    input_path: Path,
    output_path: Optional[Path],
    expected_columns: Optional[int],
    stop_markers: Sequence[str],
) -> Tuple[int, int, Optional[int]]:
    kept = 0
    skipped = 0
    resolved_expected = expected_columns
    destination = None
    header_found = False

    if output_path is not None:
        output_path.parent.mkdir(parents=True, exist_ok=True)

    try:
        with input_path.open("rb") as source:
            for raw_line in source:
                line = raw_line.decode("utf-8", errors="ignore")
                if _should_stop(line, stop_markers):
                    break
                stripped = line.strip()
                if not stripped:
                    continue
                if not header_found:
                    stripped = stripped.lstrip("\ufeff")
                    header_line: Optional[str] = None
                    if stripped == EXPECTED_HEADER:
                        header_line = EXPECTED_HEADER
                    elif stripped.startswith("#"):
                        candidate = stripped.lstrip("#").strip()
                        if candidate.lower().startswith("columns:"):
                            _, _, tail = candidate.partition(":")
                            columns = tail.strip()
                            if columns == EXPECTED_HEADER:
                                header_line = EXPECTED_HEADER
                    if header_line:
                        header_found = True
                        resolved_expected = resolved_expected or len(
                            header_line.split(",")
                        )
                        if destination is None and output_path is not None:
                            destination = output_path.open(
                                "w", encoding="utf-8", newline="\n"
                            )
                        if destination is not None:
                            destination.write(header_line + "\n")
                        kept += 1
                    continue

                cleaned, resolved_expected = _clean_numeric_row(
                    line, resolved_expected
                )
                if cleaned is None:
                    skipped += 1
                    continue
                if destination is not None:
                    destination.write(cleaned + "\n")
                elif output_path is not None:
                    destination = output_path.open("w", encoding="utf-8", newline="\n")
                    destination.write(cleaned + "\n")
                kept += 1
    finally:
        if destination is not None:
            destination.close()

    return kept, skipped, resolved_expected


def _discover_csv_files(targets: Sequence[Path]) -> List[Path]:
    files: List[Path] = []
    for target in targets:
        resolved = target.expanduser().resolve()
        if not resolved.exists():
            raise SystemExit(f"Input path {resolved} does not exist.")
        if resolved.is_dir():
            for candidate in sorted(resolved.glob("*.csv")):
                if candidate.stem.endswith("_clean"):
                    continue
                files.append(candidate)
        elif resolved.is_file():
            if resolved.stem.endswith("_clean"):
                continue
            files.append(resolved)
        else:
            raise SystemExit(f"Unsupported input target: {resolved}")

    # Deduplicate while preserving order.
    unique: List[Path] = []
    seen = set()
    for path in files:
        if path in seen:
            continue
        seen.add(path)
        unique.append(path)
    if not unique:
        raise SystemExit("No CSV files found to clean.")
    return unique


def main() -> None:
    args = parse_args()
    targets = args.input if args.input else [Path.cwd()]
    input_files = _discover_csv_files(targets)

    if args.stop_marker:
        stop_markers = [
            marker.strip().lower()
            for marker in args.stop_marker.split("|")
            if marker.strip()
        ]
    else:
        stop_markers = []

    if args.output and len(input_files) != 1:
        raise SystemExit("--output can only be used when cleaning a single file.")

    for input_path in input_files:
        if args.output:
            output_path = args.output.expanduser().resolve()
        else:
            default_name = f"{input_path.stem}_clean{input_path.suffix}"
            output_path = input_path.with_name(default_name)

        output_for_clean = None if args.dry_run else output_path
        kept, skipped, resolved_columns = clean_file(
            input_path,
            output_for_clean,
            args.expected_columns,
            stop_markers,
        )

        if kept == 0:
            message = (
                f"{input_path.name}: header '{EXPECTED_HEADER}' not found; skipped."
            )
            if args.dry_run:
                print(message)
            else:
                print(message)
            continue

        summary = (
            f"{input_path.name}: kept {kept} rows"
            f" (skipped {skipped})"
        )

        if resolved_columns is not None:
            summary += f" across {resolved_columns} columns"

        if args.dry_run:
            print(summary + ".")
        else:
            print(f"{summary}. Wrote cleaned data to {output_path}.")


if __name__ == "__main__":
    main()
