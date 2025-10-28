import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:smartfridge_app/core/theme/demo_colors.dart';
import 'package:smartfridge_app/features/analytics/analytics_v1_classic.dart';
import 'package:smartfridge_app/features/alerts/alerts_list_screen.dart';
import 'package:smartfridge_app/features/dashboard/dashboard_v4_apple_home.dart';
import 'package:smartfridge_app/features/diagnostics/diagnostics_screen.dart';

/// Main App Home with Beautiful Bottom Navigation
/// Three tabs: Dashboard, Analytics, Alerts
class AppHome extends StatefulWidget {
  const AppHome({super.key});

  @override
  State<AppHome> createState() => _AppHomeState();
}

class _AppHomeState extends State<AppHome> {
  int _currentIndex = 0;

  // The three main screens
  final List<Widget> _screens = [
    const DashboardScreen(),
    const AnalyticsV1Classic(),
    const DiagnosticsScreen(),
    const AlertsListScreen(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: IndexedStack(index: _currentIndex, children: _screens),
      bottomNavigationBar: SafeArea(
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
          color: DemoColors.background,
          child: Row(
            mainAxisAlignment: MainAxisAlignment.spaceAround,
            children: [
              _buildNavItem(
                icon: CupertinoIcons.house,
                activeIcon: CupertinoIcons.house_fill,
                label: 'Dashboard',
                index: 0,
              ),
              _buildNavItem(
                icon: CupertinoIcons.chart_bar_square,
                activeIcon: CupertinoIcons.chart_bar_square_fill,
                label: 'Analytics',
                index: 1,
              ),
              _buildNavItem(
                icon: CupertinoIcons.slider_horizontal_3,
                activeIcon: CupertinoIcons.slider_horizontal_3,
                label: 'Diagnostics',
                index: 2,
              ),
              _buildNavItem(
                icon: CupertinoIcons.bell,
                activeIcon: CupertinoIcons.bell_fill,
                label: 'Alerts',
                index: 3,
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildNavItem({
    required IconData icon,
    required IconData activeIcon,
    required String label,
    required int index,
  }) {
    final isActive = _currentIndex == index;

    return Semantics(
      label: label,
      button: true,
      selected: isActive,
      child: GestureDetector(
        onTap: () {
          setState(() {
            _currentIndex = index;
          });
        },
        behavior: HitTestBehavior.opaque,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 250),
          padding: const EdgeInsets.all(14),
          decoration: BoxDecoration(
            color: isActive ? const Color(0xFF1C1C1E) : Colors.transparent,
            shape: BoxShape.circle,
          ),
          child: Icon(
            isActive ? activeIcon : icon,
            color: isActive ? Colors.white : const Color(0xFF8E8E93),
            size: 24,
          ),
        ),
      ),
    );
  }
}

/// Dashboard Screen (wrapper for Dashboard V4)
class DashboardScreen extends StatelessWidget {
  const DashboardScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return const DashboardV4AppleHome();
  }
}
