import 'package:flutter/material.dart';
import 'package:smartfridge_app/core/theme/demo_colors.dart';
import 'package:smartfridge_app/features/alerts/alert_v2_card.dart';
import 'package:google_fonts/google_fonts.dart';

/// Alerts List Screen - Shows all alerts and anomalies
/// Uses Alert V2 card style for displaying alerts
class AlertsListScreen extends StatefulWidget {
  const AlertsListScreen({super.key});

  @override
  State<AlertsListScreen> createState() => _AlertsListScreenState();
}

class _AlertsListScreenState extends State<AlertsListScreen> {
  final List<AlertItem> _alerts = [
    AlertItem(
      title: 'Compressor Anomaly Detected',
      description: 'Unusual vibration pattern detected in compressor',
      confidence: 94,
      severity: 'High',
      timestamp: '2 hours ago',
      isNew: true,
      type: AlertType.anomaly,
    ),
    AlertItem(
      title: 'Temperature Spike',
      description: 'Temperature briefly exceeded 8°C',
      confidence: 87,
      severity: 'Medium',
      timestamp: '5 hours ago',
      isNew: false,
      type: AlertType.warning,
    ),
    AlertItem(
      title: 'Maintenance Due',
      description: 'Scheduled maintenance check required',
      confidence: 100,
      severity: 'Low',
      timestamp: '1 day ago',
      isNew: false,
      type: AlertType.maintenance,
    ),
    AlertItem(
      title: 'Energy Optimization Available',
      description: 'Switch to Eco mode to save 15% energy',
      confidence: 92,
      severity: 'Low',
      timestamp: '2 days ago',
      isNew: false,
      type: AlertType.info,
    ),
  ];

  @override
  Widget build(BuildContext context) {
    final newAlerts = _alerts.where((a) => a.isNew).toList();
    final oldAlerts = _alerts.where((a) => !a.isNew).toList();

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
                          'Alerts',
                          style: GoogleFonts.inter(
                            fontSize: 32,
                            fontWeight: FontWeight.w700,
                            color: DemoColors.text,
                            letterSpacing: -1,
                          ),
                        ),
                        Text(
                          '${_alerts.length} total • ${newAlerts.length} new',
                          style: GoogleFonts.inter(
                            fontSize: 14,
                            fontWeight: FontWeight.w500,
                            color: DemoColors.textSecondary,
                          ),
                        ),
                      ],
                    ),
                    IconButton(
                      icon: const Icon(Icons.filter_list, color: DemoColors.text),
                      onPressed: () {},
                    ),
                  ],
                ),
              ),
              Expanded(
                child: ListView(
                  padding: const EdgeInsets.all(16),
                  children: [
                    if (newAlerts.isNotEmpty) ...[
                      Text(
                        'NEW',
                        style: GoogleFonts.inter(
                          fontSize: 12,
                          fontWeight: FontWeight.w600,
                          color: DemoColors.textSecondary,
                          letterSpacing: 1,
                        ),
                      ),
                      const SizedBox(height: 12),
                      ...newAlerts.map((alert) => Padding(
                            padding: const EdgeInsets.only(bottom: 12),
                            child: _buildAlertCard(alert),
                          )),
                      const SizedBox(height: 24),
                    ],
                    if (oldAlerts.isNotEmpty) ...[
                      Text(
                        'EARLIER',
                        style: GoogleFonts.inter(
                          fontSize: 12,
                          fontWeight: FontWeight.w600,
                          color: DemoColors.textSecondary,
                          letterSpacing: 1,
                        ),
                      ),
                      const SizedBox(height: 12),
                      ...oldAlerts.map((alert) => Padding(
                            padding: const EdgeInsets.only(bottom: 12),
                            child: _buildAlertCard(alert),
                          )),
                    ],
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildAlertCard(AlertItem alert) {
    Color getAlertColor() {
      switch (alert.type) {
        case AlertType.anomaly:
          return DemoColors.error;
        case AlertType.warning:
          return DemoColors.warning;
        case AlertType.maintenance:
          return DemoColors.textSecondary;
        case AlertType.info:
          return DemoColors.success;
      }
    }

    IconData getAlertIcon() {
      switch (alert.type) {
        case AlertType.anomaly:
          return Icons.error_outline;
        case AlertType.warning:
          return Icons.warning_amber_outlined;
        case AlertType.maintenance:
          return Icons.build_outlined;
        case AlertType.info:
          return Icons.info_outline;
      }
    }

    return GestureDetector(
      onTap: () {
        AlertV2Card.show(context);
      },
      child: Container(
        padding: const EdgeInsets.all(16),
        decoration: BoxDecoration(
          color: DemoColors.surface,
          borderRadius: BorderRadius.circular(16),
          border: Border.all(
            color: alert.isNew
                ? getAlertColor().withValues(alpha: 0.3)
                : DemoColors.cardBorder,
            width: alert.isNew ? 2 : 1,
          ),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Container(
                  width: 40,
                  height: 40,
                  decoration: BoxDecoration(
                    color: getAlertColor().withValues(alpha: 0.1),
                    borderRadius: BorderRadius.circular(10),
                  ),
                  child: Icon(
                    getAlertIcon(),
                    color: getAlertColor(),
                    size: 20,
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Row(
                        children: [
                          Expanded(
                            child: Text(
                              alert.title,
                              style: GoogleFonts.inter(
                                fontSize: 16,
                                fontWeight: FontWeight.w600,
                                color: DemoColors.text,
                                letterSpacing: -0.3,
                              ),
                            ),
                          ),
                          if (alert.isNew)
                            Container(
                              padding: const EdgeInsets.symmetric(
                                horizontal: 8,
                                vertical: 4,
                              ),
                              decoration: BoxDecoration(
                                color: getAlertColor(),
                                borderRadius: BorderRadius.circular(8),
                              ),
                              child: Text(
                                'NEW',
                                style: GoogleFonts.inter(
                                  fontSize: 10,
                                  fontWeight: FontWeight.w700,
                                  color: DemoColors.surface,
                                  letterSpacing: 0.5,
                                ),
                              ),
                            ),
                        ],
                      ),
                      const SizedBox(height: 2),
                      Text(
                        alert.timestamp,
                        style: GoogleFonts.inter(
                          fontSize: 12,
                          fontWeight: FontWeight.w400,
                          color: DemoColors.textSecondary,
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            Text(
              alert.description,
              style: GoogleFonts.inter(
                fontSize: 14,
                fontWeight: FontWeight.w400,
                color: DemoColors.text,
                height: 1.4,
              ),
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                _buildMetricBadge(
                  'Confidence',
                  '${alert.confidence}%',
                ),
                const SizedBox(width: 8),
                _buildMetricBadge(
                  'Severity',
                  alert.severity,
                ),
                const Spacer(),
                Icon(
                  Icons.chevron_right,
                  color: DemoColors.textSecondary,
                  size: 20,
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildMetricBadge(String label, String value) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      decoration: BoxDecoration(
        color: DemoColors.background,
        borderRadius: BorderRadius.circular(8),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            label,
            style: GoogleFonts.inter(
              fontSize: 11,
              fontWeight: FontWeight.w500,
              color: DemoColors.textSecondary,
            ),
          ),
          const SizedBox(width: 4),
          Text(
            value,
            style: GoogleFonts.inter(
              fontSize: 11,
              fontWeight: FontWeight.w700,
              color: DemoColors.text,
            ),
          ),
        ],
      ),
    );
  }
}

/// Alert item model
class AlertItem {
  final String title;
  final String description;
  final int confidence;
  final String severity;
  final String timestamp;
  final bool isNew;
  final AlertType type;

  AlertItem({
    required this.title,
    required this.description,
    required this.confidence,
    required this.severity,
    required this.timestamp,
    required this.isNew,
    required this.type,
  });
}

enum AlertType {
  anomaly,
  warning,
  maintenance,
  info,
}
