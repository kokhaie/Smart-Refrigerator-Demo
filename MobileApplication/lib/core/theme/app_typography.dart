import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'app_colors.dart';

/// Premium typography system
/// SF Pro Display for iOS aesthetic, Inter for Android
class AppTypography {
  AppTypography._();

  // Base font family
  static String get fontFamily {
    return GoogleFonts.inter().fontFamily!;
  }

  // Display - For hero numbers only
  static TextStyle display = TextStyle(
    fontSize: 56,
    fontWeight: FontWeight.w200, // Ultralight for elegance
    letterSpacing: -2,
    fontFamily: GoogleFonts.robotoMono().fontFamily, // Monospace for numbers
    color: AppColors.black,
  );

  // Headlines
  static TextStyle h1 = TextStyle(
    fontSize: 34,
    fontWeight: FontWeight.w700,
    letterSpacing: -0.5,
    fontFamily: fontFamily,
    color: AppColors.black,
  );

  static TextStyle h2 = TextStyle(
    fontSize: 24,
    fontWeight: FontWeight.w600,
    letterSpacing: -0.5,
    fontFamily: fontFamily,
    color: AppColors.black,
  );

  static TextStyle h3 = TextStyle(
    fontSize: 20,
    fontWeight: FontWeight.w500,
    fontFamily: fontFamily,
    color: AppColors.black,
  );

  // Body
  static TextStyle body = TextStyle(
    fontSize: 16,
    fontWeight: FontWeight.w400,
    height: 1.5, // Line height for readability
    fontFamily: fontFamily,
    color: AppColors.black,
  );

  static TextStyle caption = TextStyle(
    fontSize: 12,
    fontWeight: FontWeight.w400,
    letterSpacing: 0.5,
    fontFamily: fontFamily,
    color: AppColors.gray500,
  );

  // Special
  static TextStyle button = TextStyle(
    fontSize: 16,
    fontWeight: FontWeight.w600,
    letterSpacing: 0.5,
    fontFamily: fontFamily,
    color: AppColors.black,
  );
}
