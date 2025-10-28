import 'package:flutter/material.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Alert Design V1: Full-Screen Modal
/// - Dramatic full-screen takeover
/// - Blurred background
/// - Large icon and text
/// - Clear call-to-action
class AlertV1Modal extends StatelessWidget {
  const AlertV1Modal({super.key});

  static void show(BuildContext context) {
    showGeneralDialog(
      context: context,
      barrierDismissible: true,
      barrierLabel: 'Alert',
      barrierColor: Colors.black.withValues(alpha: 0.5),
      transitionDuration: const Duration(milliseconds: 300),
      pageBuilder: (context, animation, secondaryAnimation) {
        return const AlertV1Modal();
      },
      transitionBuilder: (context, animation, secondaryAnimation, child) {
        return FadeTransition(
          opacity: animation,
          child: ScaleTransition(
            scale: Tween<double>(begin: 0.95, end: 1.0).animate(
              CurvedAnimation(parent: animation, curve: Curves.easeOutCubic),
            ),
            child: child,
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Container(
        margin: const EdgeInsets.all(AppSpacing.lg),
        padding: const EdgeInsets.all(AppSpacing.xl),
        decoration: BoxDecoration(
          color: AppColors.white,
          borderRadius: BorderRadius.circular(24),
        ),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            // Warning icon
            Container(
              width: 80,
              height: 80,
              decoration: BoxDecoration(
                color: AppColors.gray50,
                shape: BoxShape.circle,
              ),
              child: const Icon(
                Icons.warning_outlined,
                size: 40,
                color: AppColors.black,
              ),
            ),

            const SizedBox(height: AppSpacing.lg),

            // Title
            Text(
              'Anomaly Detected',
              style: AppTypography.h1,
              textAlign: TextAlign.center,
            ),

            const SizedBox(height: AppSpacing.sm),

            // Description
            Text(
              'Unusual temperature fluctuation detected in cooling system',
              style: AppTypography.body.copyWith(color: AppColors.gray700),
              textAlign: TextAlign.center,
            ),

            const SizedBox(height: AppSpacing.xl),

            // Metrics grid
            Row(
              children: [
                Expanded(child: _buildMetric('CONFIDENCE', '94%')),
                const SizedBox(width: AppSpacing.md),
                Expanded(child: _buildMetric('DETECTED', '2m ago')),
              ],
            ),

            const SizedBox(height: AppSpacing.md),

            Row(
              children: [
                Expanded(child: _buildMetric('SEVERITY', 'MEDIUM')),
                const SizedBox(width: AppSpacing.md),
                Expanded(child: _buildMetric('SAVINGS', '﷼ 150')),
              ],
            ),

            const SizedBox(height: AppSpacing.xl),

            // Actions
            SizedBox(
              width: double.infinity,
              child: ElevatedButton(
                onPressed: () => Navigator.pop(context),
                style: ElevatedButton.styleFrom(
                  backgroundColor: AppColors.black,
                  foregroundColor: AppColors.white,
                ),
                child: const Text('VIEW DETAILS'),
              ),
            ),

            const SizedBox(height: AppSpacing.sm),

            SizedBox(
              width: double.infinity,
              child: OutlinedButton(
                onPressed: () => Navigator.pop(context),
                child: const Text('DISMISS'),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildMetric(String label, String value) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.md),
      decoration: BoxDecoration(
        color: AppColors.gray50,
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
}
