import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:smartfridge_app/main.dart';

void main() {
  testWidgets('App launches successfully', (WidgetTester tester) async {
    // Build our app and trigger a frame.
    await tester.pumpWidget(const SmartFridgeApp());

    // Verify that the design selector screen loads
    expect(find.text('Design Variations'), findsOneWidget);
    expect(find.text('DASHBOARD DESIGNS'), findsOneWidget);
  });
}
