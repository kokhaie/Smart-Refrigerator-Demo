import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Analytics Design V2: Dashboard Grid
/// - Compact card-based layout
/// - Multiple metrics at-a-glance
/// - Information-dense design
/// - Perfect for quick insights
class AnalyticsV2Grid extends StatelessWidget {
  const AnalyticsV2Grid({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.white,
      appBar: AppBar(
        title: Text('Analytics', style: AppTypography.h2),
        backgroundColor: AppColors.white,
        elevation: 0,
        actions: [
          IconButton(
            icon: const Icon(Icons.calendar_today_outlined, size: 20),
            onPressed: () {},
          ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(AppSpacing.md),
        child: Column(
          children: [
            // Top stats row
            Row(
              children: [
                Expanded(
                  child: _buildMetricCard(
                    '3.5°C',
                    'AVG TEMP',
                    Icons.thermostat_outlined,
                  ),
                ),
                const SizedBox(width: AppSpacing.md),
                Expanded(
                  child: _buildMetricCard(
                    '﷼ 1.2K',
                    'SAVINGS',
                    Icons.savings_outlined,
                  ),
                ),
              ],
            ),

            const SizedBox(height: AppSpacing.md),

            Row(
              children: [
                Expanded(
                  child: _buildMetricCard(
                    '2.1 kWh',
                    'ENERGY',
                    Icons.bolt_outlined,
                  ),
                ),
                const SizedBox(width: AppSpacing.md),
                Expanded(
                  child: _buildMetricCard(
                    '99.8%',
                    'UPTIME',
                    Icons.check_circle_outline,
                  ),
                ),
              ],
            ),

            const SizedBox(height: AppSpacing.md),

            // Temperature chart card
            _buildChartCard(
              'Temperature Trend',
              '24 Hours',
              _buildMiniLineChart(),
            ),

            const SizedBox(height: AppSpacing.md),

            // Energy chart card
            _buildChartCard(
              'Energy Usage',
              '7 Days',
              _buildMiniBarChart(),
            ),

            const SizedBox(height: AppSpacing.md),

            // Two column layout
            Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Expanded(
                  child: _buildModeCard(),
                ),
                const SizedBox(width: AppSpacing.md),
                Expanded(
                  child: _buildHealthCard(),
                ),
              ],
            ),

            const SizedBox(height: AppSpacing.lg),
          ],
        ),
      ),
      bottomNavigationBar: _buildBottomNav(),
    );
  }

  Widget _buildMetricCard(String value, String label, IconData icon) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Icon(icon, size: 20, color: AppColors.gray500),
              Container(
                padding: const EdgeInsets.all(4),
                decoration: BoxDecoration(
                  color: AppColors.gray50,
                  borderRadius: BorderRadius.circular(4),
                ),
                child: Icon(
                  Icons.trending_up,
                  size: 12,
                  color: AppColors.black,
                ),
              ),
            ],
          ),
          const SizedBox(height: AppSpacing.sm),
          Text(value, style: AppTypography.h2),
          const SizedBox(height: AppSpacing.xs),
          Text(label, style: AppTypography.caption),
        ],
      ),
    );
  }

  Widget _buildChartCard(String title, String subtitle, Widget chart) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(title, style: AppTypography.h3),
              Text(subtitle, style: AppTypography.caption),
            ],
          ),
          const SizedBox(height: AppSpacing.md),
          SizedBox(height: 120, child: chart),
        ],
      ),
    );
  }

  Widget _buildMiniLineChart() {
    return LineChart(
      LineChartData(
        gridData: FlGridData(
          show: true,
          drawVerticalLine: false,
          horizontalInterval: 2,
          getDrawingHorizontalLine: (value) {
            return const FlLine(
              color: AppColors.gray100,
              strokeWidth: 1,
            );
          },
        ),
        titlesData: const FlTitlesData(show: false),
        borderData: FlBorderData(show: false),
        lineBarsData: [
          LineChartBarData(
            spots: const [
              FlSpot(0, 3.5),
              FlSpot(1, 3.2),
              FlSpot(2, 3.8),
              FlSpot(3, 4.0),
              FlSpot(4, 3.6),
              FlSpot(5, 3.3),
              FlSpot(6, 3.5),
            ],
            isCurved: true,
            curveSmoothness: 0.3,
            color: AppColors.black,
            barWidth: 2,
            isStrokeCapRound: true,
            dotData: const FlDotData(show: false),
            belowBarData: BarAreaData(
              show: true,
              color: AppColors.black.withValues(alpha: 0.05),
            ),
          ),
        ],
        minY: 0,
        maxY: 8,
      ),
    );
  }

  Widget _buildMiniBarChart() {
    return BarChart(
      BarChartData(
        alignment: BarChartAlignment.spaceAround,
        maxY: 4,
        barTouchData: BarTouchData(enabled: false),
        titlesData: const FlTitlesData(show: false),
        gridData: const FlGridData(show: false),
        borderData: FlBorderData(show: false),
        barGroups: List.generate(7, (i) {
          final values = [2.1, 1.8, 2.3, 1.9, 2.0, 2.5, 2.2];
          return BarChartGroupData(
            x: i,
            barRods: [
              BarChartRodData(
                toY: values[i],
                color: AppColors.black,
                width: 12,
                borderRadius: BorderRadius.circular(4),
              ),
            ],
          );
        }),
      ),
    );
  }

  Widget _buildModeCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.gray50,
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('MODE', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildModeRow('ECO', 0.6),
          const SizedBox(height: AppSpacing.sm),
          _buildModeRow('SMART', 0.3),
          const SizedBox(height: AppSpacing.sm),
          _buildModeRow('RAPID', 0.1),
        ],
      ),
    );
  }

  Widget _buildModeRow(String label, double value) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(label, style: AppTypography.caption),
            Text('${(value * 100).toInt()}%', style: AppTypography.caption),
          ],
        ),
        const SizedBox(height: 4),
        Container(
          height: 6,
          decoration: BoxDecoration(
            color: AppColors.white,
            borderRadius: BorderRadius.circular(3),
          ),
          child: FractionallySizedBox(
            alignment: Alignment.centerLeft,
            widthFactor: value,
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

  Widget _buildHealthCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.black,
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        children: [
          Text(
            'HEALTH',
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
          const SizedBox(height: AppSpacing.md),
          SizedBox(
            width: 70,
            height: 70,
            child: Stack(
              alignment: Alignment.center,
              children: [
                SizedBox(
                  width: 70,
                  height: 70,
                  child: CircularProgressIndicator(
                    value: 0.94,
                    strokeWidth: 2,
                    backgroundColor: AppColors.gray700,
                    valueColor: const AlwaysStoppedAnimation(AppColors.white),
                  ),
                ),
                Text(
                  '94%',
                  style: AppTypography.h3.copyWith(color: AppColors.white),
                ),
              ],
            ),
          ),
          const SizedBox(height: AppSpacing.sm),
          Text(
            'EXCELLENT',
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
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
        selectedIndex: 1,
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
