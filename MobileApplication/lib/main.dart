import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'core/theme/app_theme.dart';
import 'app_home.dart';

void main() {
  runApp(
    const ProviderScope(
      child: SmartFridgeApp(),
    ),
  );
}

class SmartFridgeApp extends StatelessWidget {
  const SmartFridgeApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Smart Fridge',
      debugShowCheckedModeBanner: false,
      theme: AppTheme.lightTheme,
      home: const AppHome(),
    );
  }
}
