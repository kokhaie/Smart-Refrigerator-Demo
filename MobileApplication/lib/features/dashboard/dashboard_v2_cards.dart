import 'package:flutter/material.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Dashboard Design V2: Card-Based Layout
/// - Organized into distinct cards
/// - Grid-style information display
/// - More compact, information-dense
/// - Premium card aesthetics with borders
class DashboardV2Cards extends StatefulWidget {
  const DashboardV2Cards({super.key});

  @override
  State<DashboardV2Cards> createState() => _DashboardV2CardsState();
}

class _DashboardV2CardsState extends State<DashboardV2Cards> {
  double temperature = 3.5;
  int selectedMode = 0;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.white,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(AppSpacing.lg),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const SizedBox(height: AppSpacing.sm),

              // Header
              Text('Smart Refrigerator', style: AppTypography.h1),
              const SizedBox(height: AppSpacing.xs),
              Text('Predictive Maintenance', style: AppTypography.caption),

              const SizedBox(height: AppSpacing.xl),

              // Temperature Card
              _buildTemperatureCard(),

              const SizedBox(height: AppSpacing.md),

              // Two-column grid
              Row(
                children: [
                  Expanded(child: _buildMotorHealthCard()),
                  const SizedBox(width: AppSpacing.md),
                  Expanded(child: _buildSavingsCard()),
                ],
              ),

              const SizedBox(height: AppSpacing.md),

              // Mode Card
              _buildModeCard(),

              const SizedBox(height: AppSpacing.md),

              // Energy Card
              _buildEnergyCard(),

              const SizedBox(height: AppSpacing.xxl),
            ],
          ),
        ),
      ),
      bottomNavigationBar: _buildBottomNav(),
    );
  }

  Widget _buildTemperatureCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: AppColors.white,
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('TEMPERATURE', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),

          Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              // Large number
              Text(
                '${temperature.toStringAsFixed(1)}°',
                style: AppTypography.display.copyWith(fontSize: 48),
              ),
              const SizedBox(width: AppSpacing.sm),
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text('CELSIUS', style: AppTypography.caption),
                  const SizedBox(height: 4),
                  Container(
                    padding: const EdgeInsets.symmetric(
                      horizontal: 8,
                      vertical: 4,
                    ),
                    decoration: BoxDecoration(
                      color: AppColors.gray50,
                      borderRadius: BorderRadius.circular(4),
                    ),
                    child: Text('OPTIMAL', style: AppTypography.caption),
                  ),
                ],
              ),
            ],
          ),

          const SizedBox(height: AppSpacing.md),

          // Slider
          SliderTheme(
            data: SliderThemeData(
              activeTrackColor: AppColors.black,
              inactiveTrackColor: AppColors.gray100,
              thumbColor: AppColors.black,
              overlayColor: AppColors.black.withValues(alpha: 0.1),
              trackHeight: 2,
              thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6),
            ),
            child: Slider(
              value: temperature,
              min: 0,
              max: 8,
              divisions: 80,
              onChanged: (value) => setState(() => temperature = value),
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
      ),
    );
  }

  Widget _buildMotorHealthCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.gray50,
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        children: [
          Text('MOTOR', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.sm),

          SizedBox(
            width: 80,
            height: 80,
            child: Stack(
              alignment: Alignment.center,
              children: [
                SizedBox(
                  width: 80,
                  height: 80,
                  child: CircularProgressIndicator(
                    value: 0.94,
                    strokeWidth: 2,
                    backgroundColor: AppColors.gray100,
                    valueColor: const AlwaysStoppedAnimation(AppColors.black),
                  ),
                ),
                Text('94%', style: AppTypography.h3),
              ],
            ),
          ),

          const SizedBox(height: AppSpacing.sm),
          Text('HEALTHY', style: AppTypography.caption),
        ],
      ),
    );
  }

  Widget _buildSavingsCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.black,
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'SAVINGS',
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
          const SizedBox(height: AppSpacing.sm),
          Text(
            '﷼ 420',
            style: AppTypography.h2.copyWith(color: AppColors.white),
          ),
          const SizedBox(height: AppSpacing.xs),
          Text(
            '15% below avg',
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
        ],
      ),
    );
  }

  Widget _buildModeCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.white,
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('OPERATION MODE', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),

          Row(
            children: [
              _buildModeChip('ECO', 0),
              const SizedBox(width: AppSpacing.sm),
              _buildModeChip('SMART', 1),
              const SizedBox(width: AppSpacing.sm),
              _buildModeChip('RAPID', 2),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildModeChip(String label, int index) {
    final isSelected = selectedMode == index;
    return Expanded(
      child: GestureDetector(
        onTap: () => setState(() => selectedMode = index),
        child: Container(
          height: 48,
          decoration: BoxDecoration(
            color: isSelected ? AppColors.black : AppColors.white,
            border: Border.all(
              color: isSelected ? AppColors.black : AppColors.gray100,
              width: isSelected ? 2 : 1,
            ),
            borderRadius: BorderRadius.circular(8),
          ),
          child: Center(
            child: Text(
              label,
              style: AppTypography.caption.copyWith(
                color: isSelected ? AppColors.white : AppColors.gray500,
                fontWeight: isSelected ? FontWeight.w600 : FontWeight.w400,
              ),
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildEnergyCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.white,
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text('ENERGY USAGE', style: AppTypography.caption),
              Text('kWh/day', style: AppTypography.caption),
            ],
          ),
          const SizedBox(height: AppSpacing.md),

          _buildEnergyBar('ECO', 0.4, '1.2'),
          const SizedBox(height: AppSpacing.sm),
          _buildEnergyBar('SMART', 0.7, '2.1'),
          const SizedBox(height: AppSpacing.sm),
          _buildEnergyBar('RAPID', 1.0, '3.0'),
        ],
      ),
    );
  }

  Widget _buildEnergyBar(String label, double percentage, String value) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(label, style: AppTypography.caption),
            Text(value, style: AppTypography.caption),
          ],
        ),
        const SizedBox(height: 4),
        Container(
          height: 6,
          decoration: BoxDecoration(
            color: AppColors.gray100,
            borderRadius: BorderRadius.circular(3),
          ),
          child: FractionallySizedBox(
            alignment: Alignment.centerLeft,
            widthFactor: percentage,
            child: Container(
              decoration: BoxDecoration(
                color: AppColors.black,
                borderRadius: BorderRadius.circular(3),
              ),
            ),
          ),
        ),
      ],
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
        labelBehavior: NavigationDestinationLabelBehavior.alwaysHide,
        destinations: const [
          NavigationDestination(
            icon: Icon(Icons.home_outlined),
            selectedIcon: Icon(Icons.home),
            label: 'Home',
          ),
          NavigationDestination(
            icon: Icon(Icons.bar_chart_outlined),
            selectedIcon: Icon(Icons.bar_chart),
            label: 'Analytics',
          ),
          NavigationDestination(
            icon: Icon(Icons.notifications_outlined),
            selectedIcon: Icon(Icons.notifications),
            label: 'Alerts',
          ),
          NavigationDestination(
            icon: Icon(Icons.settings_outlined),
            selectedIcon: Icon(Icons.settings),
            label: 'Settings',
          ),
        ],
      ),
    );
  }
}
