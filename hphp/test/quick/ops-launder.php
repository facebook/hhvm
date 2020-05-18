<?hh
<<__EntryPoint>> function main(): void {
print "\"Hello\" . \" world!\" --> "; print __hhvm_intrinsics\launder_value("Hello") . " world!"; print "\n";
print "\n";

print "1 + 1 --> "; print __hhvm_intrinsics\launder_value(1) + 1; print "\n";
print "1.5 + 1 --> "; print __hhvm_intrinsics\launder_value(1.5) + 1; print "\n";
print "1 + 1.5 --> "; print __hhvm_intrinsics\launder_value(1) + 1.5; print "\n";
print "1.5 + 1.0 --> "; print __hhvm_intrinsics\launder_value(1.5) + 1.0; print "\n";
#print "print array(1, 2) + array(3, 4) --> "; print array(1, 2) + array(3, 4);
#  print "\n";
print "\"5.5\" + 5 --> "; print __hhvm_intrinsics\launder_value("5.5") + 5; print "\n";
print "5 + \"5.5\" --> "; print __hhvm_intrinsics\launder_value(5) + "5.5"; print "\n";
print "5.5 + \"5\" --> "; print __hhvm_intrinsics\launder_value(5.5) + "5"; print "\n";
print "\"5.5\" + \"5\" --> "; print __hhvm_intrinsics\launder_value("5.5") + "5"; print "\n";
print "\n";

print "1 - 1 --> "; print __hhvm_intrinsics\launder_value(1) - 1; print "\n";
print "1.5 - 1 --> "; print __hhvm_intrinsics\launder_value(1.5) - 1; print "\n";
print "1 - 1.5 --> "; print __hhvm_intrinsics\launder_value(1) - 1.5; print "\n";
print "1.5 - 1.0 --> "; print __hhvm_intrinsics\launder_value(1.5) - 1.0; print "\n";
print "\"5.5\" - 5 --> "; print __hhvm_intrinsics\launder_value("5.5") - 5; print "\n";
print "5 - \"5.5\" --> "; print __hhvm_intrinsics\launder_value(5) - "5.5"; print "\n";
print "5.5 - \"5\" --> "; print __hhvm_intrinsics\launder_value(5.5) - "5"; print "\n";
print "\"5.5\" - \"5\" --> "; print __hhvm_intrinsics\launder_value("5.5") - "5"; print "\n";
print "\n";

print "2 * 2 --> "; print __hhvm_intrinsics\launder_value(2) * 2; print "\n";
print "2.5 * 3 --> "; print __hhvm_intrinsics\launder_value(2.5) * 3; print "\n";
print "3 * 2.5 --> "; print __hhvm_intrinsics\launder_value(3) * 2.5; print "\n";
print "2.5 * 3.0 --> "; print __hhvm_intrinsics\launder_value(2.5) * 3.0; print "\n";
print "\"5.5\" * 5 --> "; print __hhvm_intrinsics\launder_value("5.5") * 5; print "\n";
print "5 * \"5.5\" --> "; print __hhvm_intrinsics\launder_value(5) * "5.5"; print "\n";
print "5.5 * \"5\" --> "; print __hhvm_intrinsics\launder_value(5.5) * "5"; print "\n";
print "\"5.5\" * \"5\" --> "; print __hhvm_intrinsics\launder_value("5.5") * "5"; print "\n";
print "\n";

print "2 / 2 --> "; print __hhvm_intrinsics\launder_value(2) / 2; print "\n";
print "2.5 / 5 --> "; print __hhvm_intrinsics\launder_value(2.5) / 5; print "\n";
print "5 / 2.0 --> "; print __hhvm_intrinsics\launder_value(5) / 2.0; print "\n";
print "5.0 / 2.0 --> "; print __hhvm_intrinsics\launder_value(5.0) / 2.0; print "\n";
print "\"5.5\" / 5 --> "; print __hhvm_intrinsics\launder_value("5.5") / 5; print "\n";
print "5 / \"5.5\" --> "; print __hhvm_intrinsics\launder_value(5) / "5.5"; print "\n";
print "5.5 / \"5\" --> "; print __hhvm_intrinsics\launder_value(5.5) / "5"; print "\n";
print "\"5.5\" / \"5\" --> "; print __hhvm_intrinsics\launder_value("5.5") / "5"; print "\n";
try {
  print "1 / 0 --> "; print __hhvm_intrinsics\launder_value(1) / 0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
try {
  print "1.0 / 0 --> "; print __hhvm_intrinsics\launder_value(1.0) / 0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
try {
  print "1 / 0.0 --> "; print __hhvm_intrinsics\launder_value(1) / 0.0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
try {
  print "1.0 / 0.0 --> "; print __hhvm_intrinsics\launder_value(1.0) / 0.0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
print "\n";

for ($i = -10; $i <= 10; $i++) {
  print $i." % 4 --> ";
  print $i % __hhvm_intrinsics\launder_value(4);
  print "\n";
}

for ($i = -10; $i <= 10; $i++) {
  print $i." % -4 --> ";
  print $i % __hhvm_intrinsics\launder_value(-4);
  print "\n";
}

print "7 % 3 --> "; print __hhvm_intrinsics\launder_value(7) % 3; print "\n";
print "-7 % 3 --> "; print __hhvm_intrinsics\launder_value(-7) % 3; print "\n";
print "7 % -3 --> "; print __hhvm_intrinsics\launder_value(7) % -3; print "\n";
print "-7 % -3 --> "; print __hhvm_intrinsics\launder_value(-7) % -3; print "\n";
print "7 % -1 --> "; print __hhvm_intrinsics\launder_value(7) % -1; print "\n";
print "7 % 1 --> "; print __hhvm_intrinsics\launder_value(7) % 1; print "\n";
print "2147483647 % 2147483647 --> "; print __hhvm_intrinsics\launder_value(2147483647) % 2147483647; print "\n";
print "123 % 2147483647 --> "; print __hhvm_intrinsics\launder_value(123) % 2147483647; print "\n";
print "10 % -2147483648 --> "; print __hhvm_intrinsics\launder_value(10) % -2147483648; print "\n";
print "2 % 2 --> "; print __hhvm_intrinsics\launder_value(2) % 2; print "\n";
print "2.5 % 5 --> "; print __hhvm_intrinsics\launder_value(2.5) % 5; print "\n";
print "5 % 2.0 --> "; print __hhvm_intrinsics\launder_value(5) % 2.0; print "\n";
print "5.0 % 2.0 --> "; print __hhvm_intrinsics\launder_value(5.0) % 2.0; print "\n";
print "\"5.5\" % 5 --> "; print __hhvm_intrinsics\launder_value("5.5") % 5; print "\n";
print "5 % \"5.5\" --> "; print __hhvm_intrinsics\launder_value(5) % "5.5"; print "\n";
print "5.5 % \"5\" --> "; print __hhvm_intrinsics\launder_value(5.5) % "5"; print "\n";
print "\"5.5\" % \"5\" --> "; print __hhvm_intrinsics\launder_value("5.5") % "5"; print "\n";
try {
  print "1 % 0 --> "; print __hhvm_intrinsics\launder_value(1) % 0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
try {
  print "1.0 % 0 --> "; print __hhvm_intrinsics\launder_value(1.0) % 0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
try {
  print "1 % 0.0 --> "; print __hhvm_intrinsics\launder_value(1) % 0.0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
try {
  print "1.0 % 0.0 --> "; print __hhvm_intrinsics\launder_value(1.0) % 0.0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
print "\n";

print "5 & 3 --> "; print __hhvm_intrinsics\launder_value(5) & 3; print "\n";
print "5.0 & 3.0 --> "; print __hhvm_intrinsics\launder_value(5.0) & 3.0; print "\n";
print "\n";

print "5 | 3 --> "; print __hhvm_intrinsics\launder_value(5) | 3; print "\n";
print "5.0 | 3.0 --> "; print __hhvm_intrinsics\launder_value(5.0) | 3.0; print "\n";
print "\n";

print "5 ^ 3 --> "; print __hhvm_intrinsics\launder_value(5) ^ 3; print "\n";
print "5.0 ^ 3.0 --> "; print __hhvm_intrinsics\launder_value(5.0) ^ 3.0; print "\n";
print "\n";

print "5 << 1 --> "; print __hhvm_intrinsics\launder_value(5) << 1; print "\n";
print "5 << 1.0 --> "; print __hhvm_intrinsics\launder_value(5) << 1.0; print "\n";
print "5 << \"hi\" --> "; print __hhvm_intrinsics\launder_value(5) << "hi"; print "\n";
print "\n";

print "5 >> 1 --> "; print __hhvm_intrinsics\launder_value(5) >> 1; print "\n";
print "5 >> 1.0 --> "; print __hhvm_intrinsics\launder_value(5) >> 1.0; print "\n";
print "5 >> \"hi\" --> "; print __hhvm_intrinsics\launder_value(5) >> "hi"; print "\n";
print "\n";

print "!0 --> "; print __hhvm_intrinsics\launder_value(!0); print "\n";
print "!5 --> "; print __hhvm_intrinsics\launder_value(!5); print "\n";
print "!false --> "; print __hhvm_intrinsics\launder_value(!false); print "\n";
print "!\"hi\" --> "; print __hhvm_intrinsics\launder_value(!"hi"); print "\n";

print "3 === 4 --> "; print __hhvm_intrinsics\launder_value(3) === 4; print "\n";
print "3 === 3 --> "; print __hhvm_intrinsics\launder_value(3) === 3; print "\n";
print "4 === 3 --> "; print __hhvm_intrinsics\launder_value(4) === 3; print "\n";
print "\"4\" === 3 --> "; print __hhvm_intrinsics\launder_value("4") === 3; print "\n";
print "\n";

print "3 !== 4 --> "; print __hhvm_intrinsics\launder_value(3) !== 4; print "\n";
print "3 !== 3 --> "; print __hhvm_intrinsics\launder_value(3) !== 3; print "\n";
print "4 !== 3 --> "; print __hhvm_intrinsics\launder_value(4) !== 3; print "\n";
print "\"4\" !== 3 --> "; print __hhvm_intrinsics\launder_value("4") !== 3; print "\n";
print "\n";

print "3 == 4 --> "; print __hhvm_intrinsics\launder_value(3) == 4; print "\n";
print "3 == 3 --> "; print __hhvm_intrinsics\launder_value(3) == 3; print "\n";
print "4 == 3 --> "; print __hhvm_intrinsics\launder_value(4) == 3; print "\n";
print "\"4\" == 3 --> "; print __hhvm_intrinsics\launder_value("4") == 3; print "\n";
print "\n";

print "3 != 4 --> "; print __hhvm_intrinsics\launder_value(3) != 4; print "\n";
print "3 != 3 --> "; print __hhvm_intrinsics\launder_value(3) != 3; print "\n";
print "4 != 3 --> "; print __hhvm_intrinsics\launder_value(4) != 3; print "\n";
print "\"4\" != 3 --> "; print __hhvm_intrinsics\launder_value("4") != 3; print "\n";
print "\n";

print "3 < 4 --> "; print __hhvm_intrinsics\launder_value(3) < 4; print "\n";
print "3 < 3 --> "; print __hhvm_intrinsics\launder_value(3) < 3; print "\n";
print "4 < 3 --> "; print __hhvm_intrinsics\launder_value(4) < 3; print "\n";
print "\"4\" < 3 --> "; print __hhvm_intrinsics\launder_value("4") < 3; print "\n";
print "\n";

print "3 <= 4 --> "; print __hhvm_intrinsics\launder_value(3) <= 4; print "\n";
print "3 <= 3 --> "; print __hhvm_intrinsics\launder_value(3) <= 3; print "\n";
print "4 <= 3 --> "; print __hhvm_intrinsics\launder_value(4) <= 3; print "\n";
print "\"4\" <= 3 --> "; print __hhvm_intrinsics\launder_value("4") <= 3; print "\n";
print "\n";

if (__hhvm_intrinsics\launder_value(true)) {
  print "true\n";
} else {
  print "false\n";
}

if (__hhvm_intrinsics\launder_value(true) && __hhvm_intrinsics\launder_value(false)) {
  print "true && false\n";
} else {
  print "!(true && false)\n";
}

if (__hhvm_intrinsics\launder_value(false) || __hhvm_intrinsics\launder_value(true)) {
  print "false || true\n";
} else {
  print "!(false || true)\n";
}

print "3 && 4 --> "; print __hhvm_intrinsics\launder_value(3) && 4; print "\n";
print "3 && 3 --> "; print __hhvm_intrinsics\launder_value(3) && 3; print "\n";
print "4 && 3 --> "; print __hhvm_intrinsics\launder_value(4) && 3; print "\n";
print "\"4\" && 3 --> "; print __hhvm_intrinsics\launder_value("4") && 3; print "\n";
print "\n";

print "(string)42 --> "; print (string)__hhvm_intrinsics\launder_value(42); print "\n";
print "(string)\"hi\" --> "; print (string)__hhvm_intrinsics\launder_value("hi"); print "\n";
print "\n";
}
