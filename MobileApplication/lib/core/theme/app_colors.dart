import 'package:flutter/material.dart';

/// Premium monochrome color palette
/// Inspired by Apple's iOS design language and Dieter Rams' principles
class AppColors {
  AppColors._();

  // Primary Palette
  static const black = Color(0xFF000000); // Pure black for primary text/icons
  static const white = Color(0xFFFFFFFF); // Pure white backgrounds

  // Gray Scale (Use Sparingly)
  static const gray900 = Color(0xFF111111); // Near black for emphasis
  static const gray700 = Color(0xFF333333); // Secondary text
  static const gray500 = Color(0xFF666666); // Tertiary text/labels
  static const gray300 = Color(0xFF999999); // Disabled states
  static const gray100 = Color(0xFFE5E5E5); // Dividers/borders
  static const gray50 = Color(0xFFF7F7F7); // Card backgrounds

  // Semantic (Still Monochrome)
  static const success = Color(0xFF000000); // Black with ✓ icon
  static const warning = Color(0xFF000000); // Black with ⚠ icon
  static const error = Color(0xFF000000); // Black with ✗ icon
}
