import 'dart:async';
import 'dart:math' as math;

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:smartfridge_app/core/theme/app_spacing.dart';
import 'package:smartfridge_app/core/theme/app_typography.dart';
import 'package:smartfridge_app/core/theme/demo_colors.dart';

class DiagnosticsScreen extends StatefulWidget {
  const DiagnosticsScreen({super.key});

  @override
  State<DiagnosticsScreen> createState() => _DiagnosticsScreenState();
}

class _DiagnosticsScreenState extends State<DiagnosticsScreen> {
  late Timer _timer;
  DateTime _lastUpdate = DateTime.now();

  final List<_Metric> _metrics = const [
    _Metric(
      icon: CupertinoIcons.waveform_path,
      label: 'Vibration RMS',
      value: '0.08 m/s²',
      status: 'Normal',
    ),
    _Metric(
      icon: Icons.electrical_services,
      label: 'Current Draw',
      value: '245 mA',
      status: 'Normal',
    ),
    _Metric(
      icon: Icons.bolt,
      label: 'Voltage',
      value: '4.98 V',
      status: 'Stable',
    ),
    _Metric(
      icon: Icons.air,
      label: 'Fan Speed',
      value: '65%',
      status: 'Running',
    ),
    _Metric(
      icon: Icons.thermostat,
      label: 'Temperature',
      value: '3.5°C',
      status: 'Target: 4°C',
    ),
    _Metric(
      icon: Icons.water_drop,
      label: 'Humidity',
      value: '52%',
      status: 'Normal',
    ),
  ];

  @override
  void initState() {
    super.initState();
    _timer = Timer.periodic(const Duration(milliseconds: 500), (_) {
      setState(() {
        _lastUpdate = DateTime.now();
      });
    });
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }

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
                          'Diagnostics',
                          style: GoogleFonts.inter(
                            fontSize: 32,
                            fontWeight: FontWeight.w700,
                            color: DemoColors.text,
                            letterSpacing: -1,
                          ),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          'Real-time sensor monitoring',
                          style: GoogleFonts.inter(
                            fontSize: 14,
                            fontWeight: FontWeight.w500,
                            color: DemoColors.textSecondary,
                          ),
                        ),
                      ],
                    ),
                    IconButton(
                      icon: const Icon(CupertinoIcons.scope, color: DemoColors.text),
                      onPressed: () {},
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
                      _buildSensorCard(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildWaveformCard(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildCurrentPatternCard(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildSpectrumCard(),
                      const SizedBox(height: AppSpacing.xxl),
                      _buildModelInsights(),
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

  Widget _buildSensorCard() {
    final updatedAgo =
        DateTime.now().difference(_lastUpdate).inMilliseconds / 1000;
    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.06),
            blurRadius: 28,
            offset: const Offset(0, 18),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Row(
                children: [
                  const Icon(
                    Icons.science_outlined,
                    color: DemoColors.text,
                    size: 20,
                  ),
                  const SizedBox(width: 8),
                  Text(
                    'Sensor Diagnostics',
                    style: GoogleFonts.inter(
                      fontSize: 18,
                      fontWeight: FontWeight.w700,
                      color: DemoColors.text,
                      letterSpacing: -0.4,
                    ),
                  ),
                ],
              ),
              Text(
                'Live • Updated ${updatedAgo.toStringAsFixed(1)}s ago',
                style: AppTypography.caption,
              ),
            ],
          ),
          const SizedBox(height: AppSpacing.lg),
          ..._metrics.map(
            (metric) => Padding(
              padding: const EdgeInsets.only(bottom: AppSpacing.sm),
              child: Row(
                children: [
                  SizedBox(
                    width: 36,
                    child: Icon(metric.icon, color: DemoColors.text, size: 20),
                  ),
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          metric.label,
                          style: GoogleFonts.inter(
                            fontSize: 14,
                            fontWeight: FontWeight.w600,
                            color: DemoColors.text,
                          ),
                        ),
                        Text(
                          metric.status,
                          style: GoogleFonts.inter(
                            fontSize: 12,
                            fontWeight: FontWeight.w500,
                            color: DemoColors.textSecondary,
                          ),
                        ),
                      ],
                    ),
                  ),
                  Text(
                    metric.value,
                    style: GoogleFonts.inter(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: DemoColors.text,
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildWaveformCard() {
    // Simulated X/Y/Z; combine into a single magnitude vector |v|.
    final xSpots = List.generate(
      11,
      (i) => FlSpot(i.toDouble(), (0.6 * (i % 3)) + 0.2),
    );
    final ySpots = List.generate(
      11,
      (i) => FlSpot(i.toDouble(), (0.4 * ((i + 1) % 4)) + 0.3),
    );
    final zSpots = List.generate(
      11,
      (i) => FlSpot(i.toDouble(), (0.5 * ((i + 2) % 5)) + 0.4),
    );

    final magnitude = List.generate(11, (i) {
      final m = math.sqrt(
        xSpots[i].y * xSpots[i].y +
            ySpots[i].y * ySpots[i].y +
            zSpots[i].y * zSpots[i].y,
      );
      return FlSpot(i.toDouble(), m);
    });

    final double yMax =
        (magnitude.fold<double>(0, (p, e) => math.max(p, e.y)) * 1.25).clamp(
              1.0,
              double.infinity,
            )
            as double;

    return _DiagnosticChartCard(
      title: 'Vibration Magnitude',
      subtitle: 'Vector magnitude |v| over last 10s',
      height: 220,
      chart: LineChart(
        LineChartData(
          minX: 0,
          maxX: 10,
          minY: 0,
          maxY: yMax,
          gridData: FlGridData(
            show: true,
            horizontalInterval: 0.5,
            verticalInterval: 2,
            drawVerticalLine: false,
            getDrawingHorizontalLine: (value) =>
                FlLine(color: DemoColors.cardBorder, strokeWidth: 0.7),
          ),
          titlesData: FlTitlesData(
            leftTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                reservedSize: 32,
                interval: 0.5,
                getTitlesWidget: (value, meta) => Text(
                  value.toStringAsFixed(1),
                  style: AppTypography.caption,
                ),
              ),
            ),
            bottomTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                reservedSize: 28,
                interval: 2,
                getTitlesWidget: (value, meta) {
                  if (value % 2 == 0) {
                    return Text(
                      '-${(10 - value.toInt())}s',
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
          lineBarsData: [_waveformLineData(magnitude, DemoColors.text)],
        ),
      ),
      legend: Row(children: [_legendSwatch('Magnitude', DemoColors.text)]),
    );
  }

  Widget _buildCurrentPatternCard() {
    final currentSpots = List.generate(
      11,
      (i) => FlSpot(i.toDouble(), 220 + ((i % 4) * 15)),
    );
    return _DiagnosticChartCard(
      title: 'Current Draw Pattern',
      subtitle: 'Window: 10 seconds',
      height: 220,
      chart: LineChart(
        LineChartData(
          minX: 0,
          maxX: 10,
          minY: 180,
          maxY: 320,
          extraLinesData: ExtraLinesData(
            horizontalLines: [
              HorizontalLine(
                y: 210,
                color: DemoColors.text.withValues(alpha: 0.4),
                strokeWidth: 1.2,
                dashArray: const [8, 8],
                label: HorizontalLineLabel(
                  show: true,
                  alignment: Alignment.topLeft,
                  labelResolver: (_) => 'Baseline',
                  style: AppTypography.caption,
                ),
              ),
            ],
          ),
          gridData: FlGridData(
            show: true,
            horizontalInterval: 20,
            drawVerticalLine: false,
            getDrawingHorizontalLine: (value) =>
                FlLine(color: DemoColors.cardBorder, strokeWidth: 0.6),
          ),
          titlesData: FlTitlesData(
            leftTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                reservedSize: 46,
                interval: 20,
                getTitlesWidget: (value, meta) =>
                    Text('${value.toInt()}', style: AppTypography.caption),
              ),
            ),
            bottomTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                reservedSize: 28,
                interval: 2,
                getTitlesWidget: (value, meta) {
                  if (value % 2 == 0) {
                    return Text(
                      '-${(10 - value.toInt())}s',
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
          lineBarsData: [
            LineChartBarData(
              spots: currentSpots,
              isCurved: true,
              barWidth: 3,
              color: DemoColors.text,
              dotData: FlDotData(show: false),
              belowBarData: BarAreaData(
                show: true,
                color: DemoColors.text.withValues(alpha: 0.12),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildSpectrumCard() {
    final bars = List.generate(
      6,
      (index) => BarChartGroupData(
        x: index,
        barRods: [
          BarChartRodData(
            toY: [8.0, 12.0, 18.0, 22.0, 14.0, 10.0][index],
            width: 18,
            borderRadius: BorderRadius.circular(12),
            color: DemoColors.text,
          ),
        ],
      ),
    );

    return _DiagnosticChartCard(
      title: 'Frequency Spectrum',
      subtitle: 'Dominant frequency: 42 Hz (Motor rotation)',
      height: 220,
      chart: BarChart(
        BarChartData(
          maxY: 24,
          barGroups: bars,
          titlesData: FlTitlesData(
            leftTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                reservedSize: 44,
                interval: 6,
                getTitlesWidget: (value, meta) =>
                    Text('${value.toInt()} amp', style: AppTypography.caption),
              ),
            ),
            bottomTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                getTitlesWidget: (value, meta) {
                  const labels = ['0', '20', '40', '60', '80', '100+'];
                  final idx = value.toInt();
                  if (idx >= 0 && idx < labels.length) {
                    return Text(
                      '${labels[idx]} Hz',
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
            horizontalInterval: 6,
            getDrawingHorizontalLine: (value) =>
                FlLine(color: DemoColors.cardBorder, strokeWidth: 0.6),
          ),
          borderData: FlBorderData(show: false),
        ),
      ),
    );
  }

  Widget _buildModelInsights() {
    final featureRows = [
      const _FeatureRow(
        label: 'Vibration RMS',
        value: '0.08',
        status: 'Normal',
      ),
      const _FeatureRow(
        label: 'Vibration Std Dev',
        value: '0.04',
        status: 'Normal',
      ),
      const _FeatureRow(label: 'Kurtosis', value: '2.8', status: 'Normal'),
      const _FeatureRow(
        label: 'Dominant Freq',
        value: '42 Hz',
        status: 'Expected',
      ),
      const _FeatureRow(
        label: 'Current Mean',
        value: '245 mA',
        status: 'Normal',
      ),
      const _FeatureRow(label: 'Spike Count', value: '2', status: 'Normal'),
      const _FeatureRow(
        label: 'Spike Height',
        value: '15 mA',
        status: 'Normal',
      ),
      const _FeatureRow(
        label: 'Spike Duty Cycle',
        value: '0.05',
        status: 'Normal',
      ),
      const _FeatureRow(
        label: 'Spectral Centroid',
        value: '38 Hz',
        status: 'Normal',
      ),
      const _FeatureRow(
        label: 'Spectral Flatness',
        value: '0.41',
        status: 'Normal',
      ),
      const _FeatureRow(
        label: 'Phase Imbalance',
        value: '0.6°',
        status: 'Normal',
      ),
      const _FeatureRow(
        label: 'Feature Quality',
        value: '12/12',
        status: 'Complete',
      ),
    ];

    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.06),
            blurRadius: 28,
            offset: const Offset(0, 18),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              const Icon(
                Icons.memory_outlined,
                color: DemoColors.text,
                size: 20,
              ),
              const SizedBox(width: 8),
              Text(
                'ML Model Status',
                style: GoogleFonts.inter(
                  fontSize: 18,
                  fontWeight: FontWeight.w700,
                  color: DemoColors.text,
                  letterSpacing: -0.4,
                ),
              ),
            ],
          ),
          const SizedBox(height: AppSpacing.md),
          _buildSummaryRow('Last Inference', '2 seconds ago'),
          _buildSummaryRow('Inference Time', '53 μs'),
          _buildSummaryRow('Confidence', '0% (Normal)'),
          _buildSummaryRow('Features Analyzed', '12 / 12'),
          const SizedBox(height: AppSpacing.md),
          Text(
            'Feature Extraction',
            style: GoogleFonts.inter(
              fontSize: 14,
              fontWeight: FontWeight.w600,
              color: DemoColors.text,
            ),
          ),
          const SizedBox(height: AppSpacing.sm),
          ...featureRows,
          const SizedBox(height: AppSpacing.lg),
          Row(
            children: [
              Expanded(
                child: TextButton.icon(
                  onPressed: () {},
                  style: TextButton.styleFrom(
                    backgroundColor: DemoColors.text.withValues(alpha: 0.05),
                    foregroundColor: DemoColors.text,
                    padding: const EdgeInsets.symmetric(
                      vertical: 14,
                      horizontal: 18,
                    ),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(20),
                    ),
                  ),
                  icon: const Icon(CupertinoIcons.chart_bar_square),
                  label: const Text('View Historical'),
                ),
              ),
              const SizedBox(width: AppSpacing.md),
              Expanded(
                child: TextButton.icon(
                  onPressed: () {},
                  style: TextButton.styleFrom(
                    backgroundColor: DemoColors.text,
                    foregroundColor: DemoColors.surface,
                    padding: const EdgeInsets.symmetric(
                      vertical: 14,
                      horizontal: 18,
                    ),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(20),
                    ),
                  ),
                  icon: const Icon(CupertinoIcons.exclamationmark_triangle),
                  label: const Text('Trigger Test Alert'),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _legendSwatch(String label, Color color) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: 10,
          height: 10,
          decoration: BoxDecoration(
            color: color,
            borderRadius: BorderRadius.circular(6),
          ),
        ),
        const SizedBox(width: 6),
        Text(label, style: AppTypography.caption),
      ],
    );
  }

  LineChartBarData _waveformLineData(List<FlSpot> spots, Color color) {
    return LineChartBarData(
      spots: spots,
      isCurved: true,
      barWidth: 2,
      color: color,
      dotData: FlDotData(show: false),
    );
  }

  Widget _buildSummaryRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.only(bottom: AppSpacing.sm),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: GoogleFonts.inter(
              fontSize: 13,
              fontWeight: FontWeight.w500,
              color: DemoColors.textSecondary,
            ),
          ),
          Text(
            value,
            style: GoogleFonts.inter(
              fontSize: 13,
              fontWeight: FontWeight.w600,
              color: DemoColors.text,
            ),
          ),
        ],
      ),
    );
  }
}

class _DiagnosticChartCard extends StatelessWidget {
  const _DiagnosticChartCard({
    required this.title,
    required this.subtitle,
    required this.chart,
    this.legend,
    this.height = 200,
  });

  final String title;
  final String subtitle;
  final Widget chart;
  final Widget? legend;
  final double height;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.lg),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.06),
            blurRadius: 28,
            offset: const Offset(0, 18),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            title,
            style: GoogleFonts.inter(
              fontSize: 16,
              fontWeight: FontWeight.w700,
              color: DemoColors.text,
              letterSpacing: -0.3,
            ),
          ),
          const SizedBox(height: 4),
          Text(
            subtitle,
            style: GoogleFonts.inter(
              fontSize: 12,
              fontWeight: FontWeight.w500,
              color: DemoColors.textSecondary,
            ),
          ),
          const SizedBox(height: AppSpacing.md),
          SizedBox(height: height, child: chart),
          if (legend != null) ...[
            const SizedBox(height: AppSpacing.md),
            legend!,
          ],
        ],
      ),
    );
  }
}

class _Metric {
  const _Metric({
    required this.icon,
    required this.label,
    required this.value,
    required this.status,
  });

  final IconData icon;
  final String label;
  final String value;
  final String status;
}

class _FeatureRow extends StatelessWidget {
  const _FeatureRow({
    required this.label,
    required this.value,
    required this.status,
  });

  final String label;
  final String value;
  final String status;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: AppSpacing.xs),
      child: Row(
        children: [
          const Icon(
            CupertinoIcons.checkmark_seal_fill,
            size: 16,
            color: DemoColors.text,
          ),
          const SizedBox(width: 10),
          Expanded(
            child: Text(
              label,
              style: GoogleFonts.inter(
                fontSize: 13,
                fontWeight: FontWeight.w600,
                color: DemoColors.text,
              ),
            ),
          ),
          Text(
            value,
            style: GoogleFonts.inter(
              fontSize: 13,
              fontWeight: FontWeight.w600,
              color: DemoColors.text,
            ),
          ),
          const SizedBox(width: 8),
          Text(
            '[ $status ]',
            style: GoogleFonts.inter(
              fontSize: 12,
              fontWeight: FontWeight.w500,
              color: DemoColors.textSecondary,
            ),
          ),
        ],
      ),
    );
  }
}
