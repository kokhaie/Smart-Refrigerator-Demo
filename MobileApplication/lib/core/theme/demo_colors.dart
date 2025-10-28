import 'package:flutter/material.dart';

/// Demo Color Scheme
/// Strategic use of color accents on top of monochrome base
/// Perfect for presentations and demos
class DemoColors {
  DemoColors._();

  // Base Colors (Apple-style)
  static const background = Color(0xFFF5F5F7); // Apple-style light gray
  static const surface = Color(0xFFFFFFFF); // Pure white cards
  static const text = Color(0xFF000000); // Black text
  static const textSecondary = Color(0xFF8E8E93); // Gray labels

  // Strategic Color Accents (minimal, purposeful use)
  static const success = Color(0xFF1C1C1E); // Deep charcoal for “healthy”
  static const warning = Color(0xFF3A3A3C); // Dark gray for alerts
  static const error = Color(
    0xFF2C2C2E,
  ); // Slightly lighter charcoal for critical
  static const active = Color(0xFF000000); // Pure black for active modes
  static const inactive = Color(0xFFE5E5EA); // Light gray for inactive

  // Temperature Colors (subtle gradients)
  static const tempCool = Color(0xFF2C2C2E); // Cool gray
  static const tempWarm = Color(0xFF3A3A3C); // Warm gray
  static const tempNeutral = Color(0xFF1C1C1E); // Optimal charcoal

  // Chart Colors (monochrome with single accent)
  static const chartPrimary = Color(0xFF000000); // Black
  static const chartSecondary = Color(0xFF8E8E93); // Gray
  static const chartAccent = Color(0xFF1C1C1E); // Charcoal for highlights
  static const chartBackground = Color(0xFFF5F5F7); // Light gray

  // Card States
  static const cardDefault = Color(0xFFFFFFFF);
  static const cardHover = Color(0xFFF5F5F7);
  static const cardActive = Color(0xFF000000);
  static const cardBorder = Color(0xFFE5E5EA);

  // Dividers and Borders
  static const divider = Color(0xFFE5E5EA);
  static const borderLight = Color(0xFFF5F5F7);
  static const borderDark = Color(0xFF8E8E93);

  // Semantic Colors for Fridge App
  static const motorHealthy = Color(0xFF1C1C1E); // Deep charcoal
  static const motorWarning = Color(0xFF3A3A3C); // Dark gray
  static const motorCritical = Color(0xFF2C2C2E); // Slightly lighter charcoal

  static const ecoModeActive = Color(0xFF2C2C2E); // Dark gray
  static const smartModeActive = Color(0xFF000000); // Black
  static const rapidModeActive = Color(0xFF3A3A3C); // Medium-dark gray

  // Savings/Money (strategic green)
  static const savingsPositive = Color(0xFF000000); // Black for positive
  static const savingsNegative = Color(0xFF2C2C2E); // Charcoal for negative
}
