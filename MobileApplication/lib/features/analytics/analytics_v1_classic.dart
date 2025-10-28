import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:smartfridge_app/core/theme/app_colors.dart';
import 'package:smartfridge_app/core/theme/app_spacing.dart';
import 'package:smartfridge_app/core/theme/app_typography.dart';
import 'package:smartfridge_app/core/theme/demo_colors.dart';

/// Analytics screen refined for the AI refrigerator showcase.
/// - Horizontal time-range tabs
/// - Temperature trend with actual vs target + optimal shading
/// - Energy consumption mode bars
/// - Key statistics grid
/// - Optional mode distribution donut
/// - Calendar action in the header
class AnalyticsV1Classic extends StatefulWidget {
  const AnalyticsV1Classic({super.key});

  @override
  State<AnalyticsV1Classic> createState() => _AnalyticsV1ClassicState();
}

class _AnalyticsV1ClassicState extends State<AnalyticsV1Classic> {
  final List<String> _ranges = ['24H', '7D', '30D', '1Y'];
  int _selectedRange = 0;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: DemoColors.background,
      body: SafeArea(
        child: Container(
          decoration: const BoxDecoration(
            color: DemoColors.surface,
            borderRadius: BorderRadius.only(
              bottomLeft: Radius.circular(32),
              bottomRight: Radius.circular(32),
            ),
          ),
          child: Column(
            children: [
              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 16),
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          'Analytics',
                          style: GoogleFonts.inter(
                            fontSize: 32,
                            fontWeight: FontWeight.w700,
                            color: DemoColors.text,
                            letterSpacing: -1,
                          ),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          'AI-driven performance insights',
                          style: GoogleFonts.inter(
                            fontSize: 14,
                            fontWeight: FontWeight.w500,
                            color: DemoColors.textSecondary,
                          ),
                        ),
                      ],
                    ),
                    IconButton(
                      icon: const Icon(
                        Icons.calendar_month_outlined,
                        color: DemoColors.text,
                        size: 24,
                      ),
                      onPressed: () {},
                      tooltip: 'Calendar',
                    ),
                  ],
                ),
              ),
              Expanded(
                child: SingleChildScrollView(
                  padding: const EdgeInsets.symmetric(
                    horizontal: AppSpacing.lg,
                    vertical: AppSpacing.md,
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      _buildRangeSelector(),
                      const SizedBox(height: AppSpacing.xl),
                      _buildSectionHeader(
                        title: 'Temperature Trend',
                        subtitle: 'Actual vs target with optimal zone',
                      ),
                      const SizedBox(height: AppSpacing.md),
                      _buildTemperatureTrendCard(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildSectionHeader(
                        title: 'Energy Consumption',
                        subtitle: 'Daily usage by mode',
                      ),
                      const SizedBox(height: AppSpacing.md),
                      _buildEnergyConsumptionCard(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildSectionHeader(
                        title: 'Key Statistics',
                        subtitle: 'Performance summary',
                      ),
                      const SizedBox(height: AppSpacing.md),
                      _buildStatisticsGrid(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildSectionHeader(
                        title: 'Mode Distribution',
                        subtitle: 'Last 7 days',
                      ),
                      const SizedBox(height: AppSpacing.md),
                      _buildModeDistributionCard(),
                      const SizedBox(height: AppSpacing.lg),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildRangeSelector() {
    return SizedBox(
      height: 46,
      child: ListView.separated(
        scrollDirection: Axis.horizontal,
        itemCount: _ranges.length,
        separatorBuilder: (_, __) => const SizedBox(width: AppSpacing.sm),
        itemBuilder: (context, index) {
          final isSelected = _selectedRange == index;
          return GestureDetector(
            onTap: () {
              setState(() {
                _selectedRange = index;
              });
            },
            child: AnimatedContainer(
              duration: const Duration(milliseconds: 200),
              padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
              decoration: BoxDecoration(
                color: isSelected ? DemoColors.text : DemoColors.surface,
                borderRadius: BorderRadius.circular(18),
                border: Border.all(
                  color: isSelected ? DemoColors.text : DemoColors.cardBorder,
                  width: 1.2,
                ),
                boxShadow: isSelected
                    ? [
                        BoxShadow(
                          color: Colors.black.withValues(alpha: 0.12),
                          blurRadius: 20,
                          offset: const Offset(0, 10),
                        ),
                      ]
                    : [],
              ),
              child: Text(
                _ranges[index],
                style: GoogleFonts.inter(
                  fontSize: 13,
                  fontWeight: FontWeight.w600,
                  color: isSelected
                      ? DemoColors.surface
                      : DemoColors.textSecondary,
                ),
              ),
            ),
          );
        },
      ),
    );
  }

  Widget _buildSectionHeader({
    required String title,
    required String subtitle,
  }) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          title,
          style: GoogleFonts.inter(
            fontSize: 20,
            fontWeight: FontWeight.w700,
            color: DemoColors.text,
            letterSpacing: -0.4,
          ),
        ),
        const SizedBox(height: 4),
        Text(
          subtitle,
          style: GoogleFonts.inter(
            fontSize: 13,
            fontWeight: FontWeight.w500,
            color: DemoColors.textSecondary,
          ),
        ),
      ],
    );
  }

  Widget _buildTemperatureTrendCard() {
    final actualSpots = [
      const FlSpot(0, 3.2),
      const FlSpot(1, 3.4),
      const FlSpot(2, 3.6),
      const FlSpot(3, 3.8),
      const FlSpot(4, 4.0),
      const FlSpot(5, 4.2),
      const FlSpot(6, 3.9),
      const FlSpot(7, 3.5),
    ];

    final targetSpots = [const FlSpot(0, 4.0), const FlSpot(7, 4.0)];

    final lowerBand = [const FlSpot(0, 3.0), const FlSpot(7, 3.0)];

    final upperBand = [const FlSpot(0, 5.0), const FlSpot(7, 5.0)];

    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.06),
            blurRadius: 30,
            offset: const Offset(0, 18),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            height: 220,
            child: LineChart(
              LineChartData(
                minX: 0,
                maxX: 7,
                minY: 0,
                maxY: 8,
                extraLinesData: ExtraLinesData(
                  horizontalLines: [
                    HorizontalLine(
                      y: 3,
                      color: DemoColors.textSecondary.withOpacity(0.08),
                      strokeWidth: 0.5,
                    ),
                    HorizontalLine(
                      y: 5,
                      color: DemoColors.textSecondary.withOpacity(0.08),
                      strokeWidth: 0.5,
                    ),
                  ],
                ),
                gridData: FlGridData(
                  show: true,
                  horizontalInterval: 1,
                  drawVerticalLine: false,
                  getDrawingHorizontalLine: (value) =>
                      FlLine(color: DemoColors.cardBorder, strokeWidth: 0.6),
                ),
                titlesData: FlTitlesData(
                  leftTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      reservedSize: 36,
                      interval: 1,
                      getTitlesWidget: (value, meta) => Text(
                        '${value.toInt()}°',
                        style: AppTypography.caption,
                      ),
                    ),
                  ),
                  bottomTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      reservedSize: 28,
                      interval: 1,
                      getTitlesWidget: (value, meta) {
                        const labels = [
                          '00h',
                          '03h',
                          '06h',
                          '09h',
                          '12h',
                          '15h',
                          '18h',
                          '21h',
                        ];
                        final index = value.toInt();
                        if (index >= 0 && index < labels.length) {
                          return Text(
                            labels[index],
                            style: AppTypography.caption,
                          );
                        }
                        return const SizedBox.shrink();
                      },
                    ),
                  ),
                  rightTitles: const AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                  topTitles: const AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                ),
                borderData: FlBorderData(show: false),
                betweenBarsData: [
                  BetweenBarsData(
                    fromIndex: 2,
                    toIndex: 3,
                    color: DemoColors.text.withValues(alpha: 0.12),
                  ),
                ],
                lineBarsData: [
                  LineChartBarData(
                    spots: actualSpots,
                    isCurved: true,
                    barWidth: 3,
                    color: DemoColors.text,
                    dotData: FlDotData(show: false),
                    belowBarData: BarAreaData(show: false),
                  ),
                  LineChartBarData(
                    spots: targetSpots,
                    isCurved: false,
                    barWidth: 2,
                    color: DemoColors.text,
                    dashArray: const [6, 6],
                    dotData: FlDotData(show: false),
                  ),
                  LineChartBarData(
                    spots: lowerBand,
                    isCurved: false,
                    color: Colors.transparent,
                    barWidth: 0,
                    dotData: FlDotData(show: false),
                  ),
                  LineChartBarData(
                    spots: upperBand,
                    isCurved: false,
                    color: Colors.transparent,
                    barWidth: 0,
                    dotData: FlDotData(show: false),
                  ),
                ],
              ),
            ),
          ),
          const SizedBox(height: AppSpacing.md),
          Row(
            children: [
              _buildLegendDot(color: DemoColors.text, label: 'Actual'),
              const SizedBox(width: 16),
              _buildLegendDot(
                color: DemoColors.text,
                label: 'Target',
                isDashed: true,
              ),
              const SizedBox(width: 16),
              _buildLegendDot(
                color: DemoColors.text.withValues(alpha: 0.15),
                label: 'Optimal (3°-5°)',
                isFilled: true,
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildEnergyConsumptionCard() {
    final days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
    final values = [2.6, 3.2, 2.8, 3.6, 2.4, 2.1, 3.0];
    final modes = ['Eco', 'Smart', 'Smart', 'Rapid', 'Eco', 'Eco', 'Smart'];

    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.06),
            blurRadius: 30,
            offset: const Offset(0, 18),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            height: 220,
            child: BarChart(
              BarChartData(
                alignment: BarChartAlignment.spaceAround,
                maxY: 4,
                barTouchData: BarTouchData(enabled: false),
                titlesData: FlTitlesData(
                  leftTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      reservedSize: 40,
                      interval: 1,
                      getTitlesWidget: (value, meta) => Text(
                        '${value.toInt()} kWh',
                        style: AppTypography.caption,
                      ),
                    ),
                  ),
                  bottomTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      getTitlesWidget: (value, meta) {
                        final index = value.toInt();
                        if (index >= 0 && index < days.length) {
                          return Text(
                            days[index],
                            style: AppTypography.caption,
                          );
                        }
                        return const SizedBox.shrink();
                      },
                    ),
                  ),
                  rightTitles: const AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                  topTitles: const AxisTitles(
                    sideTitles: SideTitles(showTitles: false),
                  ),
                ),
                gridData: FlGridData(
                  show: true,
                  drawVerticalLine: false,
                  horizontalInterval: 1,
                  getDrawingHorizontalLine: (value) =>
                      FlLine(color: DemoColors.cardBorder, strokeWidth: 0.6),
                ),
                borderData: FlBorderData(show: false),
                barGroups: List.generate(days.length, (index) {
                  final mode = modes[index];
                  Color barColor;
                  switch (mode) {
                    case 'Eco':
                    case 'Rapid':
                    default:
                      barColor = DemoColors.text;
                  }

                  return BarChartGroupData(
                    x: index,
                    barRods: [
                      BarChartRodData(
                        toY: values[index],
                        color: barColor,
                        width: 20,
                        borderRadius: BorderRadius.circular(10),
                      ),
                    ],
                  );
                }),
              ),
            ),
          ),
          const SizedBox(height: AppSpacing.md),
          Row(
            children: [
              _buildLegendDot(
                color: DemoColors.text,
                label: 'Smart',
                isFilled: true,
              ),
              const SizedBox(width: 16),
              _buildLegendDot(
                color: DemoColors.text,
                label: 'Eco',
                isFilled: true,
              ),
              const SizedBox(width: 16),
              _buildLegendDot(
                color: DemoColors.text,
                label: 'Rapid',
                isFilled: true,
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildStatisticsGrid() {
    final cards = [
      _StatInfo(
        title: 'Avg Temperature',
        value: '3.7°',
        icon: Icons.thermostat_outlined,
        trend: '+0.3° vs target',
      ),
      _StatInfo(
        title: 'Uptime',
        value: '99.3%',
        icon: Icons.timer_outlined,
        trend: 'Stable operation',
      ),
    ];

    return GridView.builder(
      physics: const NeverScrollableScrollPhysics(),
      shrinkWrap: true,
      itemCount: cards.length,
      gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
        crossAxisCount: 2,
        crossAxisSpacing: AppSpacing.md,
        mainAxisSpacing: AppSpacing.md,
        childAspectRatio: 1.1,
      ),
      itemBuilder: (context, index) {
        final card = cards[index];
        return Container(
          padding: const EdgeInsets.all(AppSpacing.lg),
          decoration: BoxDecoration(
            color: DemoColors.surface,
            borderRadius: BorderRadius.circular(24),
            boxShadow: [
              BoxShadow(
                color: Colors.black.withValues(alpha: 0.05),
                blurRadius: 24,
                offset: const Offset(0, 14),
              ),
            ],
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Container(
                width: 44,
                height: 44,
                decoration: BoxDecoration(
                  color: DemoColors.background,
                  borderRadius: BorderRadius.circular(16),
                ),
                child: Icon(card.icon, color: DemoColors.text, size: 22),
              ),
              const Spacer(),
              Text(
                card.title,
                style: GoogleFonts.inter(
                  fontSize: 13,
                  fontWeight: FontWeight.w500,
                  color: DemoColors.textSecondary,
                ),
              ),
              const SizedBox(height: 6),
              Text(
                card.value,
                style: GoogleFonts.inter(
                  fontSize: 24,
                  fontWeight: FontWeight.w700,
                  color: DemoColors.text,
                  letterSpacing: -0.6,
                ),
              ),
              const SizedBox(height: 6),
              Text(
                card.trend,
                style: GoogleFonts.inter(
                  fontSize: 12,
                  fontWeight: FontWeight.w500,
                  color: DemoColors.textSecondary,
                ),
              ),
            ],
          ),
        );
      },
    );
  }

  Widget _buildModeDistributionCard() {
    final modeSections = [
      PieChartSectionData(
        color: DemoColors.text,
        value: 55,
        title: '55%',
        radius: 62,
        titleStyle: GoogleFonts.inter(
          fontSize: 16,
          fontWeight: FontWeight.w600,
          color: DemoColors.surface,
        ),
      ),
      PieChartSectionData(
        color: DemoColors.text.withValues(alpha: 0.7),
        value: 30,
        title: '30%',
        radius: 52,
        titleStyle: GoogleFonts.inter(
          fontSize: 15,
          fontWeight: FontWeight.w600,
          color: DemoColors.surface,
        ),
      ),
      PieChartSectionData(
        color: DemoColors.text.withValues(alpha: 0.45),
        value: 15,
        title: '15%',
        radius: 46,
        titleStyle: GoogleFonts.inter(
          fontSize: 14,
          fontWeight: FontWeight.w600,
          color: DemoColors.surface,
        ),
      ),
    ];

    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.05),
            blurRadius: 24,
            offset: const Offset(0, 14),
          ),
        ],
      ),
      child: Column(
        children: [
          SizedBox(
            height: 220,
            child: PieChart(
              PieChartData(
                sections: modeSections,
                centerSpaceRadius: 48,
                sectionsSpace: 2,
              ),
            ),
          ),
          const SizedBox(height: AppSpacing.md),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              _buildLegendDot(
                color: DemoColors.text,
                label: 'Smart',
                isFilled: true,
              ),
              _buildLegendDot(
                color: DemoColors.text.withValues(alpha: 0.7),
                label: 'Eco',
                isFilled: true,
              ),
              _buildLegendDot(
                color: DemoColors.text.withValues(alpha: 0.45),
                label: 'Rapid',
                isFilled: true,
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildLegendDot({
    required Color color,
    required String label,
    bool isDashed = false,
    bool isFilled = false,
  }) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: 12,
          height: 12,
          decoration: BoxDecoration(
            color: isFilled ? color : Colors.transparent,
            borderRadius: BorderRadius.circular(6),
            border: Border.all(color: color, width: isDashed ? 1.6 : 1.2),
          ),
        ),
        const SizedBox(width: 8),
        Text(
          label,
          style: GoogleFonts.inter(
            fontSize: 12,
            fontWeight: FontWeight.w600,
            color: DemoColors.textSecondary,
          ),
        ),
      ],
    );
  }
}

class _StatInfo {
  final String title;
  final String value;
  final String trend;
  final IconData icon;

  _StatInfo({
    required this.title,
    required this.value,
    required this.trend,
    required this.icon,
  });
}
