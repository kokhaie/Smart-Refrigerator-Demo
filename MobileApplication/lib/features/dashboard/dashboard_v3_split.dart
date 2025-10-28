import 'package:flutter/material.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Dashboard Design V3: Split-Screen Layout
/// - Bold two-tone design (black/white split)
/// - Asymmetric layout for visual interest
/// - High contrast for demo visibility
/// - Magazine-style editorial layout
class DashboardV3Split extends StatefulWidget {
  const DashboardV3Split({super.key});

  @override
  State<DashboardV3Split> createState() => _DashboardV3SplitState();
}

class _DashboardV3SplitState extends State<DashboardV3Split> {
  double temperature = 3.5;
  int selectedMode = 0;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.white,
      body: SafeArea(
        child: Column(
          children: [
            // Top hero section - Black background
            Expanded(
              flex: 5,
              child: Container(
                color: AppColors.black,
                padding: const EdgeInsets.all(AppSpacing.lg),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const SizedBox(height: AppSpacing.sm),

                    // Header
                    Text(
                      'SMART FRIDGE',
                      style:
                          AppTypography.caption.copyWith(color: AppColors.gray300),
                    ),

                    const Spacer(),

                    // Temperature Display
                    Row(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          temperature.toStringAsFixed(1),
                          style: AppTypography.display.copyWith(
                            fontSize: 72,
                            color: AppColors.white,
                          ),
                        ),
                        Padding(
                          padding: const EdgeInsets.only(top: 12),
                          child: Text(
                            '°C',
                            style: AppTypography.h1.copyWith(color: AppColors.white),
                          ),
                        ),
                      ],
                    ),

                    const SizedBox(height: AppSpacing.xs),

                    Text(
                      'TEMPERATURE',
                      style: AppTypography.caption.copyWith(color: AppColors.gray300),
                    ),

                    const SizedBox(height: AppSpacing.lg),

                    // Slider
                    SliderTheme(
                      data: SliderThemeData(
                        activeTrackColor: AppColors.white,
                        inactiveTrackColor: AppColors.gray700,
                        thumbColor: AppColors.white,
                        overlayColor: AppColors.white.withValues(alpha: 0.1),
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
                        Text(
                          '0° MIN',
                          style: AppTypography.caption.copyWith(color: AppColors.gray300),
                        ),
                        Text(
                          '8° MAX',
                          style: AppTypography.caption.copyWith(color: AppColors.gray300),
                        ),
                      ],
                    ),

                    const Spacer(),
                  ],
                ),
              ),
            ),

            // Bottom section - White background
            Expanded(
              flex: 6,
              child: Container(
                color: AppColors.white,
                child: SingleChildScrollView(
                  padding: const EdgeInsets.all(AppSpacing.lg),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const SizedBox(height: AppSpacing.sm),

                      // Status grid
                      Row(
                        children: [
                          Expanded(child: _buildStatusCard('MOTOR', '94%', 'HEALTHY')),
                          const SizedBox(width: AppSpacing.md),
                          Expanded(
                              child: _buildStatusCard('SAVINGS', '﷼ 420', 'TODAY')),
                        ],
                      ),

                      const SizedBox(height: AppSpacing.lg),

                      // Mode selector
                      Text('OPERATION MODE', style: AppTypography.caption),
                      const SizedBox(height: AppSpacing.md),

                      _buildModeSelector(),

                      const SizedBox(height: AppSpacing.lg),

                      // Energy bars
                      Text('ENERGY CONSUMPTION', style: AppTypography.caption),
                      const SizedBox(height: AppSpacing.md),

                      _buildEnergyBars(),

                      const SizedBox(height: AppSpacing.lg),
                    ],
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
      bottomNavigationBar: _buildBottomNav(),
    );
  }

  Widget _buildStatusCard(String label, String value, String subtitle) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(label, style: AppTypography.caption),
          const SizedBox(height: AppSpacing.sm),
          Text(value, style: AppTypography.h2),
          const SizedBox(height: AppSpacing.xs),
          Text(subtitle, style: AppTypography.caption),
        ],
      ),
    );
  }

  Widget _buildModeSelector() {
    return Container(
      padding: const EdgeInsets.all(4),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Row(
        children: [
          _buildModeTab('ECO', 0),
          _buildModeTab('SMART', 1),
          _buildModeTab('RAPID', 2),
        ],
      ),
    );
  }

  Widget _buildModeTab(String label, int index) {
    final isSelected = selectedMode == index;
    return Expanded(
      child: GestureDetector(
        onTap: () => setState(() => selectedMode = index),
        child: Container(
          height: 44,
          decoration: BoxDecoration(
            color: isSelected ? AppColors.black : Colors.transparent,
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

  Widget _buildEnergyBars() {
    return Column(
      children: [
        _buildEnergyRow('ECO', 0.4),
        const SizedBox(height: AppSpacing.md),
        _buildEnergyRow('SMART', 0.7),
        const SizedBox(height: AppSpacing.md),
        _buildEnergyRow('RAPID', 1.0),
      ],
    );
  }

  Widget _buildEnergyRow(String label, double percentage) {
    return Row(
      children: [
        SizedBox(
          width: 60,
          child: Text(label, style: AppTypography.caption),
        ),
        Expanded(
          child: Stack(
            children: [
              Container(
                height: 24,
                decoration: BoxDecoration(
                  border: Border.all(color: AppColors.gray100, width: 1),
                  borderRadius: BorderRadius.circular(4),
                ),
              ),
              FractionallySizedBox(
                widthFactor: percentage,
                child: Container(
                  height: 24,
                  decoration: BoxDecoration(
                    color: AppColors.black,
                    borderRadius: BorderRadius.circular(4),
                  ),
                ),
              ),
            ],
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

  Widget _buildBottomNav() {
    return Container(
      decoration: const BoxDecoration(
        border: Border(top: BorderSide(color: AppColors.gray100, width: 1)),
      ),
      child: NavigationBar(
        backgroundColor: AppColors.white,
        elevation: 0,
        selectedIndex: 0,
        indicatorColor: AppColors.black,
        labelBehavior: NavigationDestinationLabelBehavior.alwaysHide,
        destinations: const [
          NavigationDestination(
            icon: Icon(Icons.home_outlined, color: AppColors.white),
            selectedIcon: Icon(Icons.home, color: AppColors.white),
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
