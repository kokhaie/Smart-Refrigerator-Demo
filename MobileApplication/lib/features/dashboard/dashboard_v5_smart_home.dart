import 'package:flutter/material.dart';
import 'package:smartfridge_app/core/theme/app_colors.dart';
import 'package:smartfridge_app/core/theme/app_spacing.dart';
import 'package:smartfridge_app/features/dashboard/widgets/apple_slider.dart';
import 'package:smartfridge_app/features/dashboard/widgets/modern_device_card.dart';
import 'package:google_fonts/google_fonts.dart';

/// Dashboard V5: Smart Home Control Center
/// Horizontal hero layout with room selector
/// Premium Apple-inspired interface with thick sliders
class DashboardV5SmartHome extends StatefulWidget {
  const DashboardV5SmartHome({super.key});

  @override
  State<DashboardV5SmartHome> createState() => _DashboardV5SmartHomeState();
}

class _DashboardV5SmartHomeState extends State<DashboardV5SmartHome> {
  double _temperature = 21.0;
  double _humidity = 52.0;
  String _selectedRoom = 'Living Room';
  bool _smartLightsOn = false;
  bool _motorOn = true;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.white,
      body: SafeArea(
        child: Column(
          children: [
            // Header
            Padding(
              padding: const EdgeInsets.all(AppSpacing.lg),
              child: Row(
                children: [
                  IconButton(
                    icon: const Icon(Icons.arrow_back, color: AppColors.black),
                    onPressed: () => Navigator.pop(context),
                  ),
                  const Spacer(),
                  Container(
                    padding: const EdgeInsets.symmetric(
                      horizontal: 16,
                      vertical: 8,
                    ),
                    decoration: BoxDecoration(
                      border: Border.all(color: AppColors.gray100),
                      borderRadius: BorderRadius.circular(20),
                    ),
                    child: Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Container(
                          width: 6,
                          height: 6,
                          decoration: const BoxDecoration(
                            color: Colors.green,
                            shape: BoxShape.circle,
                          ),
                        ),
                        const SizedBox(width: 8),
                        Text(
                          'Connected',
                          style: GoogleFonts.inter(
                            fontSize: 12,
                            fontWeight: FontWeight.w500,
                            color: AppColors.gray700,
                          ),
                        ),
                      ],
                    ),
                  ),
                ],
              ),
            ),

            Expanded(
              child: SingleChildScrollView(
                padding: const EdgeInsets.symmetric(horizontal: AppSpacing.lg),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    // Welcome message
                    Text(
                      'Welcome Home',
                      style: GoogleFonts.inter(
                        fontSize: 32,
                        fontWeight: FontWeight.w700,
                        color: AppColors.black,
                        letterSpacing: -1,
                      ),
                    ),
                    Text(
                      'Garret Reynolds',
                      style: GoogleFonts.inter(
                        fontSize: 16,
                        fontWeight: FontWeight.w400,
                        color: AppColors.gray500,
                        letterSpacing: -0.2,
                      ),
                    ),

                    const SizedBox(height: AppSpacing.xl),

                    // Room selector tabs
                    SingleChildScrollView(
                      scrollDirection: Axis.horizontal,
                      child: Row(
                        children: [
                          _buildRoomTab('Living Room', Icons.weekend_outlined),
                          const SizedBox(width: 12),
                          _buildRoomTab('Kitchen', Icons.kitchen_outlined),
                          const SizedBox(width: 12),
                          _buildRoomTab('Dining', Icons.dining_outlined),
                          const SizedBox(width: 12),
                          _buildRoomTab('Bathroom', Icons.bathtub_outlined),
                        ],
                      ),
                    ),

                    const SizedBox(height: AppSpacing.xl),

                    // Energy card with house icon
                    Container(
                      width: double.infinity,
                      padding: const EdgeInsets.all(24),
                      decoration: BoxDecoration(
                        color: AppColors.gray50,
                        borderRadius: BorderRadius.circular(32),
                      ),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              // House icon
                              Container(
                                width: 80,
                                height: 80,
                                decoration: BoxDecoration(
                                  color: AppColors.white,
                                  borderRadius: BorderRadius.circular(20),
                                ),
                                child: CustomPaint(
                                  painter: _HouseIconPainter(),
                                ),
                              ),
                              // Energy value
                              Text(
                                '89 €',
                                style: GoogleFonts.inter(
                                  fontSize: 40,
                                  fontWeight: FontWeight.w700,
                                  color: AppColors.black,
                                  letterSpacing: -1.5,
                                ),
                              ),
                            ],
                          ),
                          const SizedBox(height: 20),
                          Text(
                            'Estimated energy',
                            style: GoogleFonts.inter(
                              fontSize: 14,
                              fontWeight: FontWeight.w500,
                              color: AppColors.gray500,
                            ),
                          ),
                          Text(
                            'expenses this month',
                            style: GoogleFonts.inter(
                              fontSize: 14,
                              fontWeight: FontWeight.w500,
                              color: AppColors.gray500,
                            ),
                          ),
                          const SizedBox(height: 20),
                          // Energy usage bar
                          Container(
                            height: 8,
                            decoration: BoxDecoration(
                              color: AppColors.white,
                              borderRadius: BorderRadius.circular(4),
                            ),
                            child: FractionallySizedBox(
                              alignment: Alignment.centerLeft,
                              widthFactor: 0.72,
                              child: Container(
                                decoration: BoxDecoration(
                                  color: AppColors.black,
                                  borderRadius: BorderRadius.circular(4),
                                ),
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),

                    const SizedBox(height: AppSpacing.xl),

                    // Smart Spotlights section
                    Text(
                      'Smart Spotlights',
                      style: GoogleFonts.inter(
                        fontSize: 20,
                        fontWeight: FontWeight.w700,
                        color: AppColors.black,
                        letterSpacing: -0.4,
                      ),
                    ),

                    const SizedBox(height: AppSpacing.md),

                    // Device control cards
                    Row(
                      children: [
                        Expanded(
                          child: _buildControlCard(
                            icon: Icons.lightbulb_outline,
                            title: 'Smart\nTV',
                            isActive: _smartLightsOn,
                            onTap: () {
                              setState(() {
                                _smartLightsOn = !_smartLightsOn;
                              });
                            },
                          ),
                        ),
                        const SizedBox(width: 16),
                        Expanded(
                          child: _buildControlCard(
                            icon: Icons.settings_input_antenna,
                            title: 'Air\nSamsung F7',
                            temperature: '21°c',
                            isActive: true,
                            onTap: () {},
                          ),
                        ),
                      ],
                    ),

                    const SizedBox(height: AppSpacing.xl),

                    // Temperature slider
                    Container(
                      padding: const EdgeInsets.all(24),
                      decoration: BoxDecoration(
                        color: AppColors.white,
                        borderRadius: BorderRadius.circular(24),
                        border: Border.all(color: AppColors.gray100, width: 1),
                      ),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              Text(
                                'Temperature',
                                style: GoogleFonts.inter(
                                  fontSize: 18,
                                  fontWeight: FontWeight.w600,
                                  color: AppColors.black,
                                  letterSpacing: -0.3,
                                ),
                              ),
                              Text(
                                '${_temperature.round()}°C',
                                style: GoogleFonts.inter(
                                  fontSize: 24,
                                  fontWeight: FontWeight.w700,
                                  color: AppColors.black,
                                  letterSpacing: -0.5,
                                ),
                              ),
                            ],
                          ),
                          const SizedBox(height: 20),
                          AppleSlider(
                            value: _temperature,
                            min: 16,
                            max: 28,
                            showValue: false,
                            onChanged: (value) {
                              setState(() {
                                _temperature = value;
                              });
                            },
                          ),
                        ],
                      ),
                    ),

                    const SizedBox(height: AppSpacing.md),

                    // Motor health
                    Container(
                      padding: const EdgeInsets.all(24),
                      decoration: BoxDecoration(
                        color: AppColors.white,
                        borderRadius: BorderRadius.circular(24),
                        border: Border.all(color: AppColors.gray100, width: 1),
                      ),
                      child: Row(
                        children: [
                          Container(
                            width: 48,
                            height: 48,
                            decoration: BoxDecoration(
                              color: AppColors.gray50,
                              borderRadius: BorderRadius.circular(12),
                            ),
                            child: const Icon(
                              Icons.settings,
                              color: AppColors.black,
                              size: 24,
                            ),
                          ),
                          const SizedBox(width: 16),
                          Expanded(
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  'Motor Health',
                                  style: GoogleFonts.inter(
                                    fontSize: 16,
                                    fontWeight: FontWeight.w600,
                                    color: AppColors.black,
                                    letterSpacing: -0.3,
                                  ),
                                ),
                                const SizedBox(height: 4),
                                Text(
                                  'Excellent • 98% efficiency',
                                  style: GoogleFonts.inter(
                                    fontSize: 13,
                                    fontWeight: FontWeight.w400,
                                    color: AppColors.gray500,
                                  ),
                                ),
                              ],
                            ),
                          ),
                          DeviceToggle(
                            value: _motorOn,
                            onChanged: (value) {
                              setState(() {
                                _motorOn = value;
                              });
                            },
                          ),
                        ],
                      ),
                    ),

                    const SizedBox(height: AppSpacing.xxl),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildRoomTab(String label, IconData icon) {
    final isSelected = _selectedRoom == label;
    return GestureDetector(
      onTap: () {
        setState(() {
          _selectedRoom = label;
        });
      },
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 200),
        padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
        decoration: BoxDecoration(
          color: isSelected ? AppColors.black : AppColors.white,
          borderRadius: BorderRadius.circular(20),
          border: Border.all(
            color: isSelected ? AppColors.black : AppColors.gray100,
            width: 1.5,
          ),
        ),
        child: Row(
          children: [
            Icon(
              icon,
              size: 18,
              color: isSelected ? AppColors.white : AppColors.gray700,
            ),
            const SizedBox(width: 8),
            Text(
              label,
              style: GoogleFonts.inter(
                fontSize: 14,
                fontWeight: FontWeight.w600,
                color: isSelected ? AppColors.white : AppColors.gray700,
                letterSpacing: -0.2,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildControlCard({
    required IconData icon,
    required String title,
    String? temperature,
    required bool isActive,
    required VoidCallback onTap,
  }) {
    return GestureDetector(
      onTap: onTap,
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 200),
        height: 160,
        padding: const EdgeInsets.all(20),
        decoration: BoxDecoration(
          color: isActive ? AppColors.black : AppColors.gray50,
          borderRadius: BorderRadius.circular(24),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Container(
                  width: 32,
                  height: 32,
                  decoration: BoxDecoration(
                    color: isActive ? AppColors.white : AppColors.white,
                    borderRadius: BorderRadius.circular(8),
                  ),
                  child: Icon(
                    icon,
                    size: 18,
                    color: AppColors.black,
                  ),
                ),
                const Spacer(),
                if (temperature != null)
                  Container(
                    padding:
                        const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                    decoration: BoxDecoration(
                      color: AppColors.white.withValues(alpha: 0.2),
                      borderRadius: BorderRadius.circular(8),
                    ),
                    child: Text(
                      temperature,
                      style: GoogleFonts.inter(
                        fontSize: 11,
                        fontWeight: FontWeight.w600,
                        color: AppColors.white,
                      ),
                    ),
                  )
                else
                  Container(
                    width: 24,
                    height: 24,
                    decoration: BoxDecoration(
                      color: isActive
                          ? AppColors.white.withValues(alpha: 0.2)
                          : AppColors.gray100,
                      shape: BoxShape.circle,
                    ),
                  ),
              ],
            ),
            const Spacer(),
            Text(
              title,
              style: GoogleFonts.inter(
                fontSize: 14,
                fontWeight: FontWeight.w600,
                color: isActive ? AppColors.white : AppColors.black,
                height: 1.3,
                letterSpacing: -0.2,
              ),
            ),
            const SizedBox(height: 4),
            Text(
              isActive ? 'Inactive' : 'Inactive',
              style: GoogleFonts.inter(
                fontSize: 11,
                fontWeight: FontWeight.w400,
                color: isActive
                    ? AppColors.white.withValues(alpha: 0.6)
                    : AppColors.gray500,
              ),
            ),
          ],
        ),
      ),
    );
  }
}

/// Custom painter for house icon
class _HouseIconPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = AppColors.black
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2.5
      ..strokeCap = StrokeCap.round
      ..strokeJoin = StrokeJoin.round;

    final path = Path();

    // House roof
    path.moveTo(size.width * 0.5, size.height * 0.25);
    path.lineTo(size.width * 0.75, size.height * 0.45);
    path.lineTo(size.width * 0.25, size.height * 0.45);
    path.close();

    // House body
    path.moveTo(size.width * 0.3, size.height * 0.45);
    path.lineTo(size.width * 0.3, size.height * 0.75);
    path.lineTo(size.width * 0.7, size.height * 0.75);
    path.lineTo(size.width * 0.7, size.height * 0.45);

    // Door
    path.moveTo(size.width * 0.45, size.height * 0.75);
    path.lineTo(size.width * 0.45, size.height * 0.6);
    path.lineTo(size.width * 0.55, size.height * 0.6);
    path.lineTo(size.width * 0.55, size.height * 0.75);

    canvas.drawPath(path, paint);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
