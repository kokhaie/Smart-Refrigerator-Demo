import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:smartfridge_app/core/theme/demo_colors.dart';
import 'package:smartfridge_app/features/dashboard/widgets/circular_temperature_dial.dart';
import 'package:smartfridge_app/features/dashboard/widgets/modern_device_card.dart';

/// Dashboard V4 redesigned with a softer, high-end smart home aesthetic.
/// Keeps existing functionality but updates layout and components to feel
/// closer to the inspiration UI.
class DashboardV4AppleHome extends StatefulWidget {
  const DashboardV4AppleHome({super.key});

  @override
  State<DashboardV4AppleHome> createState() => _DashboardV4AppleHomeState();
}

class _DashboardV4AppleHomeState extends State<DashboardV4AppleHome> {
  double _temperature = 3.5;
  String _selectedMode = 'smart';
  // Removed statistics tabs for a cleaner layout

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
          child: SingleChildScrollView(
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 24),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _buildHeader(),
                const SizedBox(height: 36),
                _buildTemperatureCard(),
                const SizedBox(height: 32),
                _buildSectionTitle(
                  'Insights',
                  subtitle: 'Keep everything on track',
                ),
                const SizedBox(height: 16),
                _buildInsightsSection(),
                const SizedBox(height: 24),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildHeader() {
    return Padding(
      padding: const EdgeInsets.only(top: 8),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  'AI Smart Refrigerator',
                  style: GoogleFonts.inter(
                    fontSize: 32,
                    fontWeight: FontWeight.w700,
                    color: DemoColors.text,
                    letterSpacing: -0.8,
                  ),
                ),
                const SizedBox(height: 10),
                Text(
                  'Adaptive cooling demo for next-gen appliances.',
                  style: GoogleFonts.inter(
                    fontSize: 14,
                    fontWeight: FontWeight.w500,
                    color: DemoColors.textSecondary,
                    letterSpacing: -0.1,
                  ),
                ),
                const SizedBox(height: 18),
                Wrap(
                  spacing: 12,
                  runSpacing: 12,
                  children: [
                    _buildStatusChip(
                      label: 'AI Optimized',
                      icon: Icons.auto_awesome,
                    ),
                    _buildConnectionStatus(),
                  ],
                ),
              ],
            ),
          ),
          const SizedBox(width: 16),
          Container(
            width: 72,
            height: 72,
            decoration: BoxDecoration(
              color: DemoColors.surface,
              borderRadius: BorderRadius.circular(24),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withValues(alpha: 0.07),
                  blurRadius: 24,
                  offset: const Offset(0, 14),
                ),
              ],
            ),
            child: const Icon(
              Icons.kitchen_outlined,
              color: DemoColors.text,
              size: 34,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildTemperatureCard() {
    return Container(
      padding: const EdgeInsets.all(28),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(36),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withValues(alpha: 0.06),
            blurRadius: 40,
            offset: const Offset(0, 24),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Temperature',
            style: GoogleFonts.inter(
              fontSize: 22,
              fontWeight: FontWeight.w700,
              color: DemoColors.text,
              letterSpacing: -0.4,
            ),
          ),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.all(18),
            decoration: BoxDecoration(
              gradient: const LinearGradient(
                colors: [Color(0xFFF9F9FB), Color(0xFFF1F1F5)],
                begin: Alignment.topCenter,
                end: Alignment.bottomCenter,
              ),
              borderRadius: BorderRadius.circular(28),
            ),
            child: Center(
              child: CircularTemperatureDial(
                temperature: _temperature,
                minTemp: 0.0,
                maxTemp: 8.0,
                mode: _getModeLabel(),
                onTemperatureChanged: (temp) {
                  setState(() {
                    _temperature = temp;
                  });
                },
              ),
            ),
          ),
          const SizedBox(height: 24),
          Row(
            children: [
              Expanded(
                child: _buildQuickStat(
                  label: 'Current temp.',
                  value: _temperature.toStringAsFixed(1),
                  unit: '°C',
                ),
              ),
              const SizedBox(width: 18),
              Expanded(
                child: _buildQuickStat(
                  label: 'Current humidity',
                  value: '52',
                  unit: '%',
                ),
              ),
            ],
          ),
          const SizedBox(height: 24),
          _buildModeSelector(),
        ],
      ),
    );
  }

  // Removed tab selector for simplicity following the UX reference

  Widget _buildQuickStat({
    required String label,
    required String value,
    required String unit,
  }) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          label,
          style: GoogleFonts.inter(
            fontSize: 13,
            fontWeight: FontWeight.w500,
            color: DemoColors.textSecondary,
            letterSpacing: -0.1,
          ),
        ),
        const SizedBox(height: 6),
        Row(
          crossAxisAlignment: CrossAxisAlignment.end,
          children: [
            Text(
              value,
              style: GoogleFonts.inter(
                fontSize: 32,
                fontWeight: FontWeight.w700,
                color: DemoColors.text,
                letterSpacing: -0.8,
              ),
            ),
            const SizedBox(width: 4),
            Text(
              unit,
              style: GoogleFonts.inter(
                fontSize: 18,
                fontWeight: FontWeight.w500,
                color: DemoColors.textSecondary,
              ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildModeSelector() {
    return Wrap(
      spacing: 12,
      runSpacing: 12,
      children: [
        ModeChip(
          label: 'Smart',
          isSelected: _selectedMode == 'smart',
          onTap: () {
            setState(() {
              _selectedMode = 'smart';
            });
          },
        ),
        ModeChip(
          label: 'Eco',
          isSelected: _selectedMode == 'eco',
          onTap: () {
            setState(() {
              _selectedMode = 'eco';
            });
          },
        ),
        ModeChip(
          label: 'Rapid',
          isSelected: _selectedMode == 'rapid',
          onTap: () {
            setState(() {
              _selectedMode = 'rapid';
            });
          },
        ),
      ],
    );
  }

  Widget _buildInsightsSection() {
    return Column(
      children: [
        _buildInsightCard(
          icon: Icons.monetization_on_outlined,
          title: 'Energy savings',
          subtitle: '420 \$ saved this month compared to average',
          color: DemoColors.success,
          onTap: () {},
        ),
        const SizedBox(height: 14),
        _buildInsightCard(
          icon: Icons.warning_amber_outlined,
          title: 'Maintenance alert',
          subtitle: 'Filter check recommended in 14 days',
          color: DemoColors.warning,
          onTap: () {},
        ),
      ],
    );
  }

  Widget _buildSectionTitle(String title, {String? subtitle}) {
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
        if (subtitle != null) ...[
          const SizedBox(height: 4),
          Text(
            subtitle,
            style: GoogleFonts.inter(
              fontSize: 13,
              fontWeight: FontWeight.w500,
              color: DemoColors.textSecondary,
              letterSpacing: -0.1,
            ),
          ),
        ],
      ],
    );
  }

  Widget _buildStatusChip({required String label, required IconData icon}) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 10),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(22),
        border: Border.all(color: DemoColors.cardBorder, width: 1),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(icon, size: 18, color: DemoColors.text),
          const SizedBox(width: 8),
          Text(
            label,
            style: GoogleFonts.inter(
              fontSize: 13,
              fontWeight: FontWeight.w600,
              color: DemoColors.text,
              letterSpacing: -0.1,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildConnectionStatus() {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
      decoration: BoxDecoration(
        color: DemoColors.surface,
        borderRadius: BorderRadius.circular(22),
        border: Border.all(color: DemoColors.cardBorder, width: 1),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 10,
            height: 10,
            decoration: const BoxDecoration(
              // explicit green dot to communicate connectivity
              color: Color(0xFF34C759),
              shape: BoxShape.circle,
            ),
          ),
          const SizedBox(width: 8),
          Text(
            'Connected',
            style: GoogleFonts.inter(
              fontSize: 13,
              fontWeight: FontWeight.w600,
              color: DemoColors.text,
              letterSpacing: -0.1,
            ),
          ),
        ],
      ),
    );
  }

  String _getModeLabel() {
    switch (_selectedMode) {
      case 'smart':
        return 'Smart';
      case 'eco':
        return 'Eco';
      case 'rapid':
        return 'Rapid';
      default:
        return 'Normal';
    }
  }

  Widget _buildInsightCard({
    required IconData icon,
    required String title,
    required String subtitle,
    required Color color,
    required VoidCallback onTap,
  }) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.all(24),
        decoration: BoxDecoration(
          color: DemoColors.surface,
          borderRadius: BorderRadius.circular(32),
          boxShadow: [
            BoxShadow(
              color: Colors.black.withValues(alpha: 0.05),
              blurRadius: 28,
              offset: const Offset(0, 18),
            ),
          ],
        ),
        child: Row(
          children: [
            Container(
              width: 56,
              height: 56,
              decoration: BoxDecoration(
                color: color.withValues(alpha: 0.12),
                borderRadius: BorderRadius.circular(18),
              ),
              child: Icon(icon, color: color, size: 26),
            ),
            const SizedBox(width: 18),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    title,
                    style: GoogleFonts.inter(
                      fontSize: 16,
                      fontWeight: FontWeight.w600,
                      color: DemoColors.text,
                      letterSpacing: -0.3,
                    ),
                  ),
                  const SizedBox(height: 6),
                  Text(
                    subtitle,
                    style: GoogleFonts.inter(
                      fontSize: 13,
                      fontWeight: FontWeight.w400,
                      color: DemoColors.textSecondary,
                    ),
                  ),
                ],
              ),
            ),
            const Icon(
              Icons.chevron_right_rounded,
              size: 22,
              color: DemoColors.textSecondary,
            ),
          ],
        ),
      ),
    );
  }
}
