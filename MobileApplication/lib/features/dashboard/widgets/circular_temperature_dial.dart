import 'dart:math' as math;
import 'package:flutter/material.dart';
import 'package:smartfridge_app/core/theme/app_colors.dart';
import 'package:google_fonts/google_fonts.dart';

/// Premium circular temperature dial with draggable control
/// Inspired by modern smart home apps with Apple design language
class CircularTemperatureDial extends StatefulWidget {
  final double temperature;
  final double minTemp;
  final double maxTemp;
  final ValueChanged<double>? onTemperatureChanged;
  final String mode;

  const CircularTemperatureDial({
    super.key,
    required this.temperature,
    this.minTemp = 0.0,
    this.maxTemp = 8.0,
    this.onTemperatureChanged,
    this.mode = 'Medium',
  });

  @override
  State<CircularTemperatureDial> createState() =>
      _CircularTemperatureDialState();
}

class _CircularTemperatureDialState extends State<CircularTemperatureDial> {
  late double _currentTemp;
  ScrollHoldController? _holdController;

  @override
  void initState() {
    super.initState();
    _currentTemp = widget.temperature;
  }

  @override
  void dispose() {
    _holdController?.cancel();
    super.dispose();
  }

  @override
  void didUpdateWidget(CircularTemperatureDial oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.temperature != oldWidget.temperature) {
      _currentTemp = widget.temperature;
    }
  }

  void _updateTemperature(Offset localPosition, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final angle = math.atan2(
      localPosition.dy - center.dy,
      localPosition.dx - center.dx,
    );

    // Arc spans the TOP semicircle: from left (-π) to right (0)
    const double startAngle = -math.pi;
    const double totalSweep = math.pi;

    // Accept only taps along the top semicircle
    if (angle < startAngle || angle > 0) {
      return;
    }

    double progress = (angle - startAngle) / totalSweep;
    progress = progress.clamp(0.0, 1.0);

    final double newTemp =
        widget.minTemp + (widget.maxTemp - widget.minTemp) * progress;

    setState(() {
      _currentTemp = newTemp.clamp(widget.minTemp, widget.maxTemp);
    });

    widget.onTemperatureChanged?.call(_currentTemp);
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      behavior: HitTestBehavior.opaque,
      onPanStart: (details) {
        _toggleScroll(false);
        final RenderBox box = context.findRenderObject() as RenderBox;
        _updateTemperature(details.localPosition, box.size);
      },
      onPanUpdate: (details) {
        final RenderBox box = context.findRenderObject() as RenderBox;
        _updateTemperature(details.localPosition, box.size);
      },
      onPanEnd: (_) => _toggleScroll(true),
      onPanCancel: () => _toggleScroll(true),
      onTapDown: (details) {
        final RenderBox box = context.findRenderObject() as RenderBox;
        _updateTemperature(details.localPosition, box.size);
      },
      child: Container(
        width: 300,
        height: 300,
        decoration: BoxDecoration(
          shape: BoxShape.circle,
          gradient: const RadialGradient(
            colors: [Color(0xFFFFFFFF), Color(0xFFF2F2F6)],
            radius: 0.95,
          ),
          boxShadow: [
            BoxShadow(
              color: Colors.black.withValues(alpha: 0.06),
              blurRadius: 36,
              offset: const Offset(0, 20),
            ),
          ],
        ),
        child: Padding(
          padding: const EdgeInsets.all(32),
          child: Stack(
            alignment: Alignment.center,
            children: [
              Positioned.fill(
                child: CustomPaint(
                  painter: _CircularDialPainter(
                    temperature: _currentTemp,
                    minTemp: widget.minTemp,
                    maxTemp: widget.maxTemp,
                    mode: widget.mode,
                  ),
                ),
              ),
              Container(
                width: 176,
                height: 176,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  gradient: const RadialGradient(
                    colors: [Color(0xFFFFFFFF), Color(0xFFF7F7FB)],
                    radius: 0.9,
                  ),
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withValues(alpha: 0.05),
                      blurRadius: 28,
                      offset: const Offset(0, 18),
                    ),
                  ],
                ),
              ),
              Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        _currentTemp.round().toString(),
                        style: GoogleFonts.inter(
                          fontSize: 70,
                          fontWeight: FontWeight.w200,
                          color: AppColors.black,
                          height: 1.0,
                          letterSpacing: -2,
                        ),
                      ),
                      Padding(
                        padding: const EdgeInsets.only(top: 11),
                        child: Text(
                          '°C',
                          style: GoogleFonts.inter(
                            fontSize: 26,
                            fontWeight: FontWeight.w400,
                            color: AppColors.gray500,
                            height: 1.0,
                          ),
                        ),
                      ),
                    ],
                  ),
                  // Minimal center, no extra labels to match reference
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  void _toggleScroll(bool enable) {
    final position = Scrollable.maybeOf(context)?.position;
    if (position == null) return;
    if (!enable) {
      _holdController ??= position.hold(() {});
    } else {
      _holdController?.cancel();
      _holdController = null;
    }
  }
}

class _CircularDialPainter extends CustomPainter {
  final double temperature;
  final double minTemp;
  final double maxTemp;
  final String mode;

  _CircularDialPainter({
    required this.temperature,
    required this.minTemp,
    required this.maxTemp,
    required this.mode,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = (math.min(size.width, size.height) / 2) - 16;

    // Arc settings - TOP semicircle: from left (180°/π) to right (0°) = 180° arc
    const double startAngle = -math.pi; // 180° (left side)
    const double totalSweep =
        math.pi; // -180° sweep counter-clockwise to right (0°)

    final progress = (temperature - minTemp) / (maxTemp - minTemp);

    _drawClockTicks(canvas, center, radius, startAngle, totalSweep);
    _drawActiveClockTicks(
      canvas,
      center,
      radius,
      startAngle,
      totalSweep,
      progress,
    );
    _drawModeText(canvas, center, radius);
    _drawTemperatureLabels(canvas, center, radius, startAngle, totalSweep);
  }

  void _drawClockTicks(
    Canvas canvas,
    Offset center,
    double radius,
    double startAngle,
    double totalSweep,
  ) {
    final paint = Paint()
      ..color = AppColors.gray100
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.round;

    // Draw tick marks like a clock
    // Draw small ticks for each 0.5 degree
    const int totalTicks = 16; // 8 degrees * 2 = 16 half-degree ticks

    for (int i = 0; i <= totalTicks; i++) {
      final double t = i / totalTicks;
      final angle = startAngle + (totalSweep * t);

      // Check if this is a major tick (whole degree)
      final bool isMajorTick = i % 2 == 0;

      // Check if this is a labeled tick (every 2 degrees: 0, 2, 4, 6, 8)
      final bool isLabeledTick = i % 4 == 0;

      final double innerRadius = radius - 8;
      final double outerRadius = isLabeledTick
          ? radius +
                8 // Biggest for labeled numbers
          : isMajorTick
          ? radius +
                4 // Medium for whole degrees
          : radius + 2; // Small for half degrees

      paint.strokeWidth = isLabeledTick
          ? 2.5
          : isMajorTick
          ? 2.0
          : 1.0;

      final x1 = center.dx + innerRadius * math.cos(angle);
      final y1 = center.dy + innerRadius * math.sin(angle);
      final x2 = center.dx + outerRadius * math.cos(angle);
      final y2 = center.dy + outerRadius * math.sin(angle);

      canvas.drawLine(Offset(x1, y1), Offset(x2, y2), paint);
    }
  }

  void _drawActiveClockTicks(
    Canvas canvas,
    Offset center,
    double radius,
    double startAngle,
    double totalSweep,
    double progress,
  ) {
    final paint = Paint()
      ..color = AppColors.black
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.round;

    const int totalTicks = 16;
    final int activeTicks = (totalTicks * progress.clamp(0.0, 1.0)).round();
    if (activeTicks <= 0) return;

    for (int i = 0; i <= activeTicks; i++) {
      final double t = i / totalTicks;
      final angle = startAngle + (totalSweep * t);

      final bool isMajorTick = i % 2 == 0;
      final bool isLabeledTick = i % 4 == 0;

      final double innerRadius = radius - 8;
      final double outerRadius = isLabeledTick
          ? radius + 8
          : isMajorTick
          ? radius + 4
          : radius + 2;

      paint.strokeWidth = isLabeledTick
          ? 2.5
          : isMajorTick
          ? 2.0
          : 1.0;

      final x1 = center.dx + innerRadius * math.cos(angle);
      final y1 = center.dy + innerRadius * math.sin(angle);
      final x2 = center.dx + outerRadius * math.cos(angle);
      final y2 = center.dy + outerRadius * math.sin(angle);

      canvas.drawLine(Offset(x1, y1), Offset(x2, y2), paint);
    }
  }

  void _drawModeText(Canvas canvas, Offset center, double radius) {
    final textPainter = TextPainter(
      text: TextSpan(
        text: mode,
        style: GoogleFonts.inter(
          fontSize: 14,
          fontWeight: FontWeight.w600,
          color: AppColors.gray500,
          letterSpacing: 0.2,
        ),
      ),
      textDirection: TextDirection.ltr,
    )..layout();

    final textOffset = Offset(
      center.dx - textPainter.width / 2,
      center.dy - radius - 32,
    );

    textPainter.paint(canvas, textOffset);
  }

  void _drawTemperatureLabels(
    Canvas canvas,
    Offset center,
    double radius,
    double startAngle,
    double totalSweep,
  ) {
    final labelRadius = radius + 22;
    final textStyle = GoogleFonts.inter(
      fontSize: 13,
      fontWeight: FontWeight.w500,
      color: AppColors.gray500,
    );

    void paintLabel(String text, double angle) {
      final painter = TextPainter(
        text: TextSpan(text: text, style: textStyle),
        textDirection: TextDirection.ltr,
      )..layout();

      final offset = Offset(
        center.dx + labelRadius * math.cos(angle) - painter.width / 2,
        center.dy + labelRadius * math.sin(angle) - painter.height / 2,
      );
      painter.paint(canvas, offset);
    }

    // Min temp at left (180°/π)
    paintLabel('${minTemp.round()}°', math.pi);
    // Max temp at right (0°)
    paintLabel('${maxTemp.round()}°', 0);
  }

  @override
  bool shouldRepaint(_CircularDialPainter oldDelegate) {
    return oldDelegate.temperature != temperature ||
        oldDelegate.minTemp != minTemp ||
        oldDelegate.maxTemp != maxTemp ||
        oldDelegate.mode != mode;
  }
}
