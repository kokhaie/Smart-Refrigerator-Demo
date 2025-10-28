import 'package:flutter/material.dart';
import 'package:smartfridge_app/core/theme/app_colors.dart';
import 'package:smartfridge_app/core/theme/app_spacing.dart';

/// Apple-style thick slider with premium feel
/// Features: Thick track, rounded thumb, smooth animations
class AppleSlider extends StatelessWidget {
  final double value;
  final double min;
  final double max;
  final ValueChanged<double>? onChanged;
  final String? label;
  final String? unit;
  final bool showValue;

  const AppleSlider({
    super.key,
    required this.value,
    this.min = 0.0,
    this.max = 100.0,
    this.onChanged,
    this.label,
    this.unit,
    this.showValue = true,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (label != null || showValue) ...[
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              if (label != null)
                Text(
                  label!,
                  style: const TextStyle(
                    fontSize: 14,
                    fontWeight: FontWeight.w500,
                    color: AppColors.gray500,
                    letterSpacing: -0.2,
                  ),
                ),
              if (showValue)
                Text(
                  '${value.round()}${unit ?? ''}',
                  style: const TextStyle(
                    fontSize: 14,
                    fontWeight: FontWeight.w600,
                    color: AppColors.black,
                    letterSpacing: -0.2,
                  ),
                ),
            ],
          ),
          const SizedBox(height: AppSpacing.sm),
        ],
        SliderTheme(
          data: SliderThemeData(
            trackHeight: 6.0, // Thick track like Apple
            thumbShape: const RoundSliderThumbShape(
              enabledThumbRadius: 14.0, // Large thumb
              elevation: 2.0,
            ),
            overlayShape: const RoundSliderOverlayShape(
              overlayRadius: 24.0,
            ),
            activeTrackColor: AppColors.black,
            inactiveTrackColor: AppColors.gray100,
            thumbColor: AppColors.white,
            overlayColor: AppColors.black.withOpacity(0.1),
            trackShape: const RoundedRectSliderTrackShape(),
            // Add shadow to thumb for depth
            thumbSelector: null,
          ),
          child: Slider(
            value: value.clamp(min, max),
            min: min,
            max: max,
            onChanged: onChanged,
          ),
        ),
      ],
    );
  }
}

/// Custom track shape with rounded ends
class RoundedRectSliderTrackShape extends SliderTrackShape {
  const RoundedRectSliderTrackShape();

  @override
  Rect getPreferredRect({
    required RenderBox parentBox,
    Offset offset = Offset.zero,
    required SliderThemeData sliderTheme,
    bool isEnabled = false,
    bool isDiscrete = false,
  }) {
    final double trackHeight = sliderTheme.trackHeight ?? 4;
    final double trackLeft = offset.dx;
    final double trackTop =
        offset.dy + (parentBox.size.height - trackHeight) / 2;
    final double trackWidth = parentBox.size.width;
    return Rect.fromLTWH(trackLeft, trackTop, trackWidth, trackHeight);
  }

  @override
  void paint(
    PaintingContext context,
    Offset offset, {
    required RenderBox parentBox,
    required SliderThemeData sliderTheme,
    required Animation<double> enableAnimation,
    required TextDirection textDirection,
    required Offset thumbCenter,
    Offset? secondaryOffset,
    bool isDiscrete = false,
    bool isEnabled = false,
    double additionalActiveTrackHeight = 2,
  }) {
    if (sliderTheme.trackHeight == null || sliderTheme.trackHeight! <= 0) {
      return;
    }

    final Rect trackRect = getPreferredRect(
      parentBox: parentBox,
      offset: offset,
      sliderTheme: sliderTheme,
      isEnabled: isEnabled,
      isDiscrete: isDiscrete,
    );

    final double trackRadius = trackRect.height / 2;

    // Active track (filled portion)
    final Paint activePaint = Paint()
      ..color = sliderTheme.activeTrackColor ?? Colors.black;

    final Rect activeRect = Rect.fromLTRB(
      trackRect.left,
      trackRect.top,
      thumbCenter.dx,
      trackRect.bottom,
    );

    context.canvas.drawRRect(
      RRect.fromRectAndRadius(activeRect, Radius.circular(trackRadius)),
      activePaint,
    );

    // Inactive track (unfilled portion)
    final Paint inactivePaint = Paint()
      ..color = sliderTheme.inactiveTrackColor ?? Colors.grey;

    final Rect inactiveRect = Rect.fromLTRB(
      thumbCenter.dx,
      trackRect.top,
      trackRect.right,
      trackRect.bottom,
    );

    context.canvas.drawRRect(
      RRect.fromRectAndRadius(inactiveRect, Radius.circular(trackRadius)),
      inactivePaint,
    );
  }
}
