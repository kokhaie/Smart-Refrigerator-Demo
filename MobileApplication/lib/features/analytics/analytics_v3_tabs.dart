import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Analytics Design V3: Tabbed Interface
/// - Category-based navigation
/// - Focused single-metric views
/// - Deep-dive analysis per tab
/// - Swipeable content
class AnalyticsV3Tabs extends StatefulWidget {
  const AnalyticsV3Tabs({super.key});

  @override
  State<AnalyticsV3Tabs> createState() => _AnalyticsV3TabsState();
}

class _AnalyticsV3TabsState extends State<AnalyticsV3Tabs>
    with SingleTickerProviderStateMixin {
  late TabController _tabController;

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: 3, vsync: this);
  }

  @override
  void dispose() {
    _tabController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.white,
      appBar: AppBar(
        title: Text('Analytics', style: AppTypography.h2),
        backgroundColor: AppColors.white,
        elevation: 0,
        bottom: PreferredSize(
          preferredSize: const Size.fromHeight(48),
          child: Container(
            decoration: const BoxDecoration(
              border: Border(
                bottom: BorderSide(color: AppColors.gray100, width: 1),
              ),
            ),
            child: TabBar(
              controller: _tabController,
              indicatorColor: AppColors.black,
              indicatorWeight: 2,
              labelColor: AppColors.black,
              unselectedLabelColor: AppColors.gray500,
              labelStyle: AppTypography.caption.copyWith(fontWeight: FontWeight.w600),
              unselectedLabelStyle: AppTypography.caption,
              tabs: const [
                Tab(text: 'TEMPERATURE'),
                Tab(text: 'ENERGY'),
                Tab(text: 'PERFORMANCE'),
              ],
            ),
          ),
        ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: [
          _buildTemperatureTab(),
          _buildEnergyTab(),
          _buildPerformanceTab(),
        ],
      ),
      bottomNavigationBar: _buildBottomNav(),
    );
  }

  Widget _buildTemperatureTab() {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(AppSpacing.lg),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Hero metric
          _buildHeroMetric('3.5°C', 'Current Temperature', '+0.2° from yesterday'),

          const SizedBox(height: AppSpacing.xl),

          // Chart
          Text('24 HOUR TREND', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildTemperatureChart(),

          const SizedBox(height: AppSpacing.xl),

          // Stats grid
          Text('STATISTICS', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          Row(
            children: [
              Expanded(child: _buildStatBox('MIN', '2.8°C')),
              const SizedBox(width: AppSpacing.md),
              Expanded(child: _buildStatBox('MAX', '4.2°C')),
              const SizedBox(width: AppSpacing.md),
              Expanded(child: _buildStatBox('AVG', '3.5°C')),
            ],
          ),

          const SizedBox(height: AppSpacing.xl),

          // Insights
          Text('INSIGHTS', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildInsightCard(
            Icons.check_circle_outline,
            'Optimal Range',
            'Temperature has been within optimal range for 7 days',
          ),
        ],
      ),
    );
  }

  Widget _buildEnergyTab() {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(AppSpacing.lg),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Hero metric
          _buildHeroMetric('2.1 kWh', 'Daily Average', '15% below target'),

          const SizedBox(height: AppSpacing.xl),

          // Chart
          Text('7 DAY CONSUMPTION', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildEnergyChart(),

          const SizedBox(height: AppSpacing.xl),

          // Mode breakdown
          Text('BY MODE', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildModeBreakdown('ECO', 1.2, 0.6),
          const SizedBox(height: AppSpacing.sm),
          _buildModeBreakdown('SMART', 2.1, 0.3),
          const SizedBox(height: AppSpacing.sm),
          _buildModeBreakdown('RAPID', 3.0, 0.1),

          const SizedBox(height: AppSpacing.xl),

          // Cost estimate
          _buildCostCard(),
        ],
      ),
    );
  }

  Widget _buildPerformanceTab() {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(AppSpacing.lg),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Motor health
          Center(
            child: Column(
              children: [
                const SizedBox(height: AppSpacing.lg),
                SizedBox(
                  width: 160,
                  height: 160,
                  child: Stack(
                    alignment: Alignment.center,
                    children: [
                      SizedBox(
                        width: 160,
                        height: 160,
                        child: CircularProgressIndicator(
                          value: 0.94,
                          strokeWidth: 3,
                          backgroundColor: AppColors.gray100,
                          valueColor:
                              const AlwaysStoppedAnimation(AppColors.black),
                        ),
                      ),
                      Column(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          Text('94%', style: AppTypography.display.copyWith(fontSize: 42)),
                          Text('HEALTH', style: AppTypography.caption),
                        ],
                      ),
                    ],
                  ),
                ),
                const SizedBox(height: AppSpacing.md),
                Text('EXCELLENT CONDITION', style: AppTypography.h3),
                const SizedBox(height: AppSpacing.xs),
                Text('All systems operational', style: AppTypography.caption),
              ],
            ),
          ),

          const SizedBox(height: AppSpacing.xxl),

          // Metrics
          Text('METRICS', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildPerformanceMetric('Uptime', '99.8%', 1.0),
          const SizedBox(height: AppSpacing.md),
          _buildPerformanceMetric('Efficiency', '94%', 0.94),
          const SizedBox(height: AppSpacing.md),
          _buildPerformanceMetric('Cycle Count', '1,234', 0.75),

          const SizedBox(height: AppSpacing.xl),

          // Maintenance
          Text('MAINTENANCE', style: AppTypography.caption),
          const SizedBox(height: AppSpacing.md),
          _buildMaintenanceCard(
            'Next Service',
            '45 days',
            Icons.build_outlined,
          ),
          const SizedBox(height: AppSpacing.sm),
          _buildMaintenanceCard(
            'Filter Change',
            '120 days',
            Icons.air_outlined,
          ),
        ],
      ),
    );
  }

  Widget _buildHeroMetric(String value, String label, String subtitle) {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: AppColors.gray50,
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        children: [
          Text(label, style: AppTypography.caption),
          const SizedBox(height: AppSpacing.sm),
          Text(value, style: AppTypography.display.copyWith(fontSize: 42)),
          const SizedBox(height: AppSpacing.xs),
          Text(subtitle, style: AppTypography.caption),
        ],
      ),
    );
  }

  Widget _buildTemperatureChart() {
    return Container(
      height: 180,
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(16),
      ),
      child: LineChart(
        LineChartData(
          gridData: FlGridData(
            show: true,
            drawVerticalLine: false,
            horizontalInterval: 2,
            getDrawingHorizontalLine: (value) {
              return const FlLine(color: AppColors.gray100, strokeWidth: 1);
            },
          ),
          titlesData: const FlTitlesData(show: false),
          borderData: FlBorderData(show: false),
          lineBarsData: [
            LineChartBarData(
              spots: List.generate(24, (i) {
                return FlSpot(i.toDouble(), 3.0 + (i % 4) * 0.5);
              }),
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
      ),
    );
  }

  Widget _buildEnergyChart() {
    return Container(
      height: 180,
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(16),
      ),
      child: BarChart(
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
                  width: 20,
                  borderRadius: BorderRadius.circular(4),
                ),
              ],
            );
          }),
        ),
      ),
    );
  }

  Widget _buildStatBox(String label, String value) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        children: [
          Text(label, style: AppTypography.caption),
          const SizedBox(height: AppSpacing.xs),
          Text(value, style: AppTypography.h3),
        ],
      ),
    );
  }

  Widget _buildInsightCard(IconData icon, String title, String description) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Row(
        children: [
          Icon(icon, size: 24, color: AppColors.black),
          const SizedBox(width: AppSpacing.md),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(title, style: AppTypography.h3),
                const SizedBox(height: AppSpacing.xs),
                Text(description, style: AppTypography.caption),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildModeBreakdown(String mode, double kwh, double percentage) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Row(
        children: [
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(mode, style: AppTypography.h3),
                const SizedBox(height: AppSpacing.xs),
                Text('$kwh kWh/day', style: AppTypography.caption),
              ],
            ),
          ),
          Text('${(percentage * 100).toInt()}%', style: AppTypography.h2),
        ],
      ),
    );
  }

  Widget _buildCostCard() {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: AppColors.black,
        borderRadius: BorderRadius.circular(16),
      ),
      child: Column(
        children: [
          Text(
            'ESTIMATED MONTHLY COST',
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
          const SizedBox(height: AppSpacing.md),
          Text(
            '﷼ 12,600',
            style: AppTypography.display
                .copyWith(fontSize: 36, color: AppColors.white),
          ),
          const SizedBox(height: AppSpacing.xs),
          Text(
            '15% below average',
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
        ],
      ),
    );
  }

  Widget _buildPerformanceMetric(String label, String value, double progress) {
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
              Text(label, style: AppTypography.h3),
              Text(value, style: AppTypography.h3),
            ],
          ),
          const SizedBox(height: AppSpacing.sm),
          ClipRRect(
            borderRadius: BorderRadius.circular(2),
            child: LinearProgressIndicator(
              value: progress,
              backgroundColor: AppColors.gray100,
              valueColor: const AlwaysStoppedAnimation(AppColors.black),
              minHeight: 4,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildMaintenanceCard(String title, String value, IconData icon) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.gray50,
        border: Border.all(color: AppColors.gray100, width: 1),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Row(
        children: [
          Icon(icon, size: 24, color: AppColors.black),
          const SizedBox(width: AppSpacing.md),
          Expanded(child: Text(title, style: AppTypography.h3)),
          Text(value, style: AppTypography.caption),
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
