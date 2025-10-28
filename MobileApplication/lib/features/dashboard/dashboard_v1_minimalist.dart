import 'package:flutter/material.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Dashboard Design V1: Ultra Minimalist
/// - Maximum whitespace
/// - Center-aligned hero element
/// - Circular gauge design
/// - Clean segmented controls
class DashboardV1Minimalist extends StatefulWidget {
  const DashboardV1Minimalist({super.key});

  @override
  State<DashboardV1Minimalist> createState() => _DashboardV1MinimalistState();
}

class _DashboardV1MinimalistState extends State<DashboardV1Minimalist> {
  double temperature = 3.5;
  int selectedMode = 0; // 0: ECO, 1: SMART, 2: RAPID

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.white,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.symmetric(horizontal: AppSpacing.lg),
          child: Column(
            children: [
              const SizedBox(height: AppSpacing.xxl),

              // Temperature Hero Section
              _buildTemperatureHero(),

              const SizedBox(height: AppSpacing.xxl),

              // Divider
              const Divider(color: AppColors.gray100, height: 1),

              const SizedBox(height: AppSpacing.xxl),

              // Motor Health
              _buildMotorHealth(),

              const SizedBox(height: AppSpacing.xxl),

              // Divider
              const Divider(color: AppColors.gray100, height: 1),

              const SizedBox(height: AppSpacing.xxl),

              // Mode Selector
              _buildModeSelector(),

              const SizedBox(height: AppSpacing.lg),

              // Energy Consumption
              _buildEnergyConsumption(),

              const SizedBox(height: AppSpacing.xxl),

              // Divider
              const Divider(color: AppColors.gray100, height: 1),

              const SizedBox(height: AppSpacing.xxl),

              // Savings Card
              _buildSavingsCard(),

              const SizedBox(height: AppSpacing.xxl),
            ],
          ),
        ),
      ),
      bottomNavigationBar: _buildBottomNav(),
    );
  }

  Widget _buildTemperatureHero() {
    return Column(
      children: [
        Text('TEMPERATURE', style: AppTypography.caption),
        const SizedBox(height: AppSpacing.md),

        // Circular Gauge
        SizedBox(
          width: 200,
          height: 200,
          child: Stack(
            alignment: Alignment.center,
            children: [
              // Background circle
              CustomPaint(
                size: const Size(200, 200),
                painter: _CircularGaugePainter(
                  value: temperature,
                  max: 8.0,
                ),
              ),

              // Center text
              Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text(
                    '${temperature.toStringAsFixed(1)}°',
                    style: AppTypography.display,
                  ),
                  Text('CELSIUS', style: AppTypography.caption),
                ],
              ),
            ],
          ),
        ),

        const SizedBox(height: AppSpacing.lg),

        // Slider
        SliderTheme(
          data: SliderThemeData(
            activeTrackColor: AppColors.black,
            inactiveTrackColor: AppColors.gray100,
            thumbColor: AppColors.black,
            overlayColor: AppColors.black.withOpacity(0.1),
            trackHeight: 2,
            thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6),
          ),
          child: Slider(
            value: temperature,
            min: 0,
            max: 8,
            divisions: 80,
            onChanged: (value) {
              setState(() {
                temperature = value;
              });
            },
          ),
        ),

        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text('0°', style: AppTypography.caption),
            Text('8°', style: AppTypography.caption),
          ],
        ),
      ],
    );
  }

  Widget _buildMotorHealth() {
    const healthPercentage = 0.94;
    return Column(
      children: [
        Text('MOTOR HEALTH', style: AppTypography.caption),
        const SizedBox(height: AppSpacing.md),
        SizedBox(
          width: 120,
          height: 120,
          child: Stack(
            alignment: Alignment.center,
            children: [
              SizedBox(
                width: 120,
                height: 120,
                child: CircularProgressIndicator(
                  value: healthPercentage,
                  strokeWidth: 2,
                  backgroundColor: AppColors.gray100,
                  valueColor: const AlwaysStoppedAnimation(AppColors.black),
                ),
              ),
              Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text('94%', style: AppTypography.h2),
                  Text('HEALTHY', style: AppTypography.caption),
                ],
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildModeSelector() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('MODE', style: AppTypography.caption),
        const SizedBox(height: AppSpacing.md),
        Row(
          children: [
            _buildModeButton('ECO', 0),
            const SizedBox(width: AppSpacing.sm),
            _buildModeButton('SMART', 1),
            const SizedBox(width: AppSpacing.sm),
            _buildModeButton('RAPID', 2),
          ],
        ),
      ],
    );
  }

  Widget _buildModeButton(String label, int index) {
    final isSelected = selectedMode == index;
    return Expanded(
      child: GestureDetector(
        onTap: () => setState(() => selectedMode = index),
        child: Container(
          height: 56,
          decoration: BoxDecoration(
            color: isSelected ? AppColors.black : AppColors.white,
            border: Border.all(
              color: AppColors.black,
              width: 2,
            ),
            borderRadius: BorderRadius.circular(12),
          ),
          child: Center(
            child: Text(
              label,
              style: AppTypography.button.copyWith(
                color: isSelected ? AppColors.white : AppColors.black,
              ),
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildEnergyConsumption() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('ENERGY CONSUMPTION', style: AppTypography.caption),
        const SizedBox(height: AppSpacing.md),
        _buildConsumptionBar('ECO', 0.4),
        const SizedBox(height: AppSpacing.sm),
        _buildConsumptionBar('SMART', 0.7),
        const SizedBox(height: AppSpacing.sm),
        _buildConsumptionBar('RAPID', 1.0),
      ],
    );
  }

  Widget _buildConsumptionBar(String label, double percentage) {
    return Row(
      children: [
        SizedBox(
          width: 60,
          child: Text(label, style: AppTypography.caption),
        ),
        Expanded(
          child: Container(
            height: 8,
            decoration: BoxDecoration(
              color: AppColors.gray100,
              borderRadius: BorderRadius.circular(4),
            ),
            child: FractionallySizedBox(
              alignment: Alignment.centerLeft,
              widthFactor: percentage,
              child: Container(
                decoration: BoxDecoration(
                  color: AppColors.black,
                  borderRadius: BorderRadius.circular(4),
                ),
              ),
            ),
          ),
        ),
        const SizedBox(width: AppSpacing.sm),
        SizedBox(
          width: 40,
          child: Text(
            '${(percentage * 100).toInt()}%',
            style: AppTypography.caption,
            textAlign: TextAlign.right,
          ),
        ),
      ],
    );
  }

  Widget _buildSavingsCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: AppColors.gray50,
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        children: [
          Text('TODAY\'S SAVINGS', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.sm),
          Text('﷼ 420', style: AppTypography.h1),
          const SizedBox(height: AppSpacing.xs),
          Text('15% below average', style: AppTypography.caption),
        ],
      ),
    );
  }

  Widget _buildBottomNav() {
    return Container(
      decoration: const BoxDecoration(
        border: Border(top: BorderSide(color: AppColors.gray100, width: 1)),
      ),
      child: NavigationBar(
        backgroundColor: AppColors.white,
        elevation: 0,
        selectedIndex: 0,
        indicatorColor: AppColors.gray50,
        destinations: const [
          NavigationDestination(
            icon: Icon(Icons.home_outlined),
            selectedIcon: Icon(Icons.home),
            label: '',
          ),
          NavigationDestination(
            icon: Icon(Icons.bar_chart_outlined),
            selectedIcon: Icon(Icons.bar_chart),
            label: '',
          ),
          NavigationDestination(
            icon: Icon(Icons.notifications_outlined),
            selectedIcon: Icon(Icons.notifications),
            label: '',
          ),
          NavigationDestination(
            icon: Icon(Icons.settings_outlined),
            selectedIcon: Icon(Icons.settings),
            label: '',
          ),
        ],
      ),
    );
  }
}

class _CircularGaugePainter extends CustomPainter {
  final double value;
  final double max;

  _CircularGaugePainter({required this.value, required this.max});

  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = size.width / 2;
    const startAngle = -240 * 3.14159 / 180;
    const sweepAngle = 300 * 3.14159 / 180;

    // Background arc
    final bgPaint = Paint()
      ..color = AppColors.gray100
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2
      ..strokeCap = StrokeCap.round;

    canvas.drawArc(
      Rect.fromCircle(center: center, radius: radius - 10),
      startAngle,
      sweepAngle,
      false,
      bgPaint,
    );

    // Value arc
    final valuePaint = Paint()
      ..color = AppColors.black
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2
      ..strokeCap = StrokeCap.round;

    final valueAngle = sweepAngle * (value / max);
    canvas.drawArc(
      Rect.fromCircle(center: center, radius: radius - 10),
      startAngle,
      valueAngle,
      false,
      valuePaint,
    );
  }

  @override
  bool shouldRepaint(_CircularGaugePainter oldDelegate) {
    return oldDelegate.value != value;
  }
}
