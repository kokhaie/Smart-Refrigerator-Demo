import 'package:flutter/material.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Alert Design V2: Card-Style Overlay
/// - Less intrusive than modal
/// - Slides up from bottom
/// - Compact information display
/// - Quick action buttons
class AlertV2Card extends StatelessWidget {
  const AlertV2Card({super.key});

  static void show(BuildContext context) {
    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.transparent,
      isScrollControlled: true,
      builder: (context) => const AlertV2Card(),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.white,
        borderRadius: BorderRadius.circular(24),
      ),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          // Handle bar
          Container(
            margin: const EdgeInsets.only(top: AppSpacing.sm),
            width: 40,
            height: 4,
            decoration: BoxDecoration(
              color: AppColors.gray100,
              borderRadius: BorderRadius.circular(2),
            ),
          ),

          Padding(
            padding: const EdgeInsets.all(AppSpacing.lg),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Header with icon
                Row(
                  children: [
                    Container(
                      width: 48,
                      height: 48,
                      decoration: BoxDecoration(
                        color: AppColors.black,
                        borderRadius: BorderRadius.circular(12),
                      ),
                      child: const Icon(
                        Icons.warning_amber_outlined,
                        color: AppColors.white,
                        size: 24,
                      ),
                    ),
                    const SizedBox(width: AppSpacing.md),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text('Anomaly Detected', style: AppTypography.h2),
                          const SizedBox(height: 2),
                          Text('2 minutes ago', style: AppTypography.caption),
                        ],
                      ),
                    ),
                    IconButton(
                      icon: const Icon(Icons.close, size: 20),
                      onPressed: () => Navigator.pop(context),
                      color: AppColors.gray500,
                    ),
                  ],
                ),

                const SizedBox(height: AppSpacing.lg),

                // Description
                Text(
                  'Unusual temperature fluctuation detected',
                  style: AppTypography.body.copyWith(color: AppColors.gray700),
                ),

                const SizedBox(height: AppSpacing.lg),

                // Divider
                const Divider(color: AppColors.gray100, height: 1),

                const SizedBox(height: AppSpacing.lg),

                // Metrics row
                Row(
                  children: [
                    _buildInlineMetric('Confidence', '94%'),
                    const SizedBox(width: AppSpacing.xl),
                    _buildInlineMetric('Severity', 'MEDIUM'),
                    const SizedBox(width: AppSpacing.xl),
                    _buildInlineMetric('Savings', '﷼ 150'),
                  ],
                ),

                const SizedBox(height: AppSpacing.lg),

                // Divider
                const Divider(color: AppColors.gray100, height: 1),

                const SizedBox(height: AppSpacing.lg),

                // Action buttons
                Row(
                  children: [
                    Expanded(
                      child: OutlinedButton(
                        onPressed: () => Navigator.pop(context),
                        style: OutlinedButton.styleFrom(
                          minimumSize: const Size(0, 48),
                        ),
                        child: const Text('DISMISS'),
                      ),
                    ),
                    const SizedBox(width: AppSpacing.sm),
                    Expanded(
                      flex: 2,
                      child: ElevatedButton(
                        onPressed: () => Navigator.pop(context),
                        style: ElevatedButton.styleFrom(
                          minimumSize: const Size(0, 48),
                        ),
                        child: const Text('VIEW DETAILS'),
                      ),
                    ),
                  ],
                ),

                SizedBox(height: MediaQuery.of(context).padding.bottom + AppSpacing.sm),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildInlineMetric(String label, String value) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(label, style: AppTypography.caption),
        const SizedBox(height: 4),
        Text(value, style: AppTypography.body.copyWith(fontWeight: FontWeight.w600)),
      ],
    );
  }
}
