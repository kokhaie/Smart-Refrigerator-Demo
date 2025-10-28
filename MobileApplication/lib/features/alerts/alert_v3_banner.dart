import 'package:flutter/material.dart';
import '../../core/theme/app_colors.dart';
import '../../core/theme/app_typography.dart';
import '../../core/theme/app_spacing.dart';

/// Alert Design V3: Expandable Banner
/// - Minimal initial state
/// - Expands on tap for details
/// - Non-blocking notification
/// - Slides in from top
class AlertV3Banner extends StatefulWidget {
  const AlertV3Banner({super.key});

  static void show(BuildContext context) {
    showDialog(
      context: context,
      barrierColor: Colors.transparent,
      barrierDismissible: true,
      builder: (context) => const AlertV3Banner(),
    );
  }

  @override
  State<AlertV3Banner> createState() => _AlertV3BannerState();
}

class _AlertV3BannerState extends State<AlertV3Banner>
    with SingleTickerProviderStateMixin {
  bool _isExpanded = false;
  late AnimationController _controller;
  late Animation<double> _expandAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 300),
      vsync: this,
    );
    _expandAnimation = CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOutCubic,
    );
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _toggleExpanded() {
    setState(() {
      _isExpanded = !_isExpanded;
      if (_isExpanded) {
        _controller.forward();
      } else {
        _controller.reverse();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Align(
      alignment: Alignment.topCenter,
      child: Material(
        color: Colors.transparent,
        child: Container(
          margin: EdgeInsets.only(
            top: MediaQuery.of(context).padding.top + AppSpacing.sm,
            left: AppSpacing.md,
            right: AppSpacing.md,
          ),
          decoration: BoxDecoration(
            color: AppColors.black,
            borderRadius: BorderRadius.circular(16),
            boxShadow: [
              BoxShadow(
                color: Colors.black.withValues(alpha: 0.2),
                blurRadius: 20,
                offset: const Offset(0, 10),
              ),
            ],
          ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              // Collapsed state - always visible
              InkWell(
                onTap: _toggleExpanded,
                borderRadius: BorderRadius.circular(16),
                child: Padding(
                  padding: const EdgeInsets.all(AppSpacing.md),
                  child: Row(
                    children: [
                      // Icon
                      Container(
                        width: 40,
                        height: 40,
                        decoration: BoxDecoration(
                          color: AppColors.white,
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: const Icon(
                          Icons.warning_amber_rounded,
                          color: AppColors.black,
                          size: 20,
                        ),
                      ),

                      const SizedBox(width: AppSpacing.md),

                      // Text
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Text(
                              'Anomaly Detected',
                              style: AppTypography.h3.copyWith(
                                color: AppColors.white,
                              ),
                            ),
                            const SizedBox(height: 2),
                            Text(
                              '2 minutes ago',
                              style: AppTypography.caption.copyWith(
                                color: AppColors.gray300,
                              ),
                            ),
                          ],
                        ),
                      ),

                      // Expand icon
                      AnimatedRotation(
                        turns: _isExpanded ? 0.5 : 0,
                        duration: const Duration(milliseconds: 300),
                        child: Icon(
                          Icons.keyboard_arrow_down,
                          color: AppColors.white,
                        ),
                      ),
                    ],
                  ),
                ),
              ),

              // Expanded content
              SizeTransition(
                sizeFactor: _expandAnimation,
                child: Column(
                  children: [
                    const Divider(
                      color: AppColors.gray700,
                      height: 1,
                      indent: AppSpacing.md,
                      endIndent: AppSpacing.md,
                    ),

                    Padding(
                      padding: const EdgeInsets.all(AppSpacing.md),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          // Description
                          Text(
                            'Unusual temperature fluctuation detected in cooling system',
                            style: AppTypography.body.copyWith(
                              color: AppColors.gray300,
                            ),
                          ),

                          const SizedBox(height: AppSpacing.md),

                          // Metrics
                          Row(
                            children: [
                              Expanded(
                                child: _buildMetric('CONFIDENCE', '94%'),
                              ),
                              const SizedBox(width: AppSpacing.sm),
                              Expanded(
                                child: _buildMetric('SEVERITY', 'MEDIUM'),
                              ),
                            ],
                          ),

                          const SizedBox(height: AppSpacing.sm),

                          Row(
                            children: [
                              Expanded(
                                child: _buildMetric('DETECTED', '2m ago'),
                              ),
                              const SizedBox(width: AppSpacing.sm),
                              Expanded(
                                child: _buildMetric('SAVINGS', '﷼ 150'),
                              ),
                            ],
                          ),

                          const SizedBox(height: AppSpacing.md),

                          // Actions
                          Row(
                            children: [
                              Expanded(
                                child: OutlinedButton(
                                  onPressed: () => Navigator.pop(context),
                                  style: OutlinedButton.styleFrom(
                                    foregroundColor: AppColors.white,
                                    side: const BorderSide(
                                      color: AppColors.white,
                                      width: 2,
                                    ),
                                    minimumSize: const Size(0, 44),
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
                                    backgroundColor: AppColors.white,
                                    foregroundColor: AppColors.black,
                                    minimumSize: const Size(0, 44),
                                  ),
                                  child: const Text('VIEW DETAILS'),
                                ),
                              ),
                            ],
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildMetric(String label, String value) {
    return Container(
      padding: const EdgeInsets.all(AppSpacing.sm),
      decoration: BoxDecoration(
        color: AppColors.gray900,
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            label,
            style: AppTypography.caption.copyWith(color: AppColors.gray300),
          ),
          const SizedBox(height: 2),
          Text(
            value,
            style: AppTypography.body.copyWith(
              color: AppColors.white,
              fontWeight: FontWeight.w600,
            ),
          ),
        ],
      ),
    );
  }
}
