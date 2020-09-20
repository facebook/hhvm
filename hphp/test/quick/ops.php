<?hh
<<__EntryPoint>> function main(): void {
print "\"Hello\" . \" world!\" --> "; print "Hello" . " world!"; print "\n";
print "\n";

print "1 + 1 --> "; print 1 + 1; print "\n";
print "1.5 + 1 --> "; print 1.5 + 1; print "\n";
print "1 + 1.5 --> "; print 1 + 1.5; print "\n";
print "1.5 + 1.0 --> "; print 1.5 + 1.0; print "\n";
#print "print array(1, 2) + array(3, 4) --> "; print array(1, 2) + array(3, 4);
#  print "\n";
print "\"5.5\" + 5 --> "; print "5.5" + 5; print "\n";
print "5 + \"5.5\" --> "; print 5 + "5.5"; print "\n";
print "5.5 + \"5\" --> "; print 5.5 + "5"; print "\n";
print "\"5.5\" + \"5\" --> "; print "5.5" + "5"; print "\n";
print "\n";

print "1 - 1 --> "; print 1 - 1; print "\n";
print "1.5 - 1 --> "; print 1.5 - 1; print "\n";
print "1 - 1.5 --> "; print 1 - 1.5; print "\n";
print "1.5 - 1.0 --> "; print 1.5 - 1.0; print "\n";
print "\"5.5\" - 5 --> "; print "5.5" - 5; print "\n";
print "5 - \"5.5\" --> "; print 5 - "5.5"; print "\n";
print "5.5 - \"5\" --> "; print 5.5 - "5"; print "\n";
print "\"5.5\" - \"5\" --> "; print "5.5" - "5"; print "\n";
print "\n";

print "2 * 2 --> "; print 2 * 2; print "\n";
print "2.5 * 3 --> "; print 2.5 * 3; print "\n";
print "3 * 2.5 --> "; print 3 * 2.5; print "\n";
print "2.5 * 3.0 --> "; print 2.5 * 3.0; print "\n";
print "\"5.5\" * 5 --> "; print "5.5" * 5; print "\n";
print "5 * \"5.5\" --> "; print 5 * "5.5"; print "\n";
print "5.5 * \"5\" --> "; print 5.5 * "5"; print "\n";
print "\"5.5\" * \"5\" --> "; print "5.5" * "5"; print "\n";
print "\n";

print "2 / 2 --> "; print 2 / 2; print "\n";
print "2.5 / 5 --> "; print 2.5 / 5; print "\n";
print "5 / 2.0 --> "; print 5 / 2.0; print "\n";
print "5.0 / 2.0 --> "; print 5.0 / 2.0; print "\n";
print "\"5.5\" / 5 --> "; print "5.5" / 5; print "\n";
print "5 / \"5.5\" --> "; print 5 / "5.5"; print "\n";
print "5.5 / \"5\" --> "; print 5.5 / "5"; print "\n";
print "\"5.5\" / \"5\" --> "; print "5.5" / "5"; print "\n";
try {
  print "1 / 0 --> "; print 1 / 0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
try {
  print "1.0 / 0 --> "; print 1.0 / 0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
try {
  print "1 / 0.0 --> "; print 1 / 0.0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
try {
  print "1.0 / 0.0 --> "; print 1.0 / 0.0;
} catch (DivisionByZeroException $e) {
  print "Division by zero\n";
}
print "\n";

for ($i = -10; $i <= 10; $i++) {
  print $i." % 4 --> ";
  print $i % 4;
  print "\n";
}

for ($i = -10; $i <= 10; $i++) {
  print $i." % -4 --> ";
  print $i % -4;
  print "\n";
}

print "7 % 3 --> "; print 7 % 3; print "\n";
print "-7 % 3 --> "; print -7 % 3; print "\n";
print "7 % -3 --> "; print 7 % -3; print "\n";
print "-7 % -3 --> "; print -7 % -3; print "\n";
print "7 % -1 --> "; print 7 % -1; print "\n";
print "7 % 1 --> "; print 7 % 1; print "\n";
print "2147483647 % 2147483647 --> "; print 2147483647 % 2147483647; print "\n";
print "123 % 2147483647 --> "; print 123 % 2147483647; print "\n";
print "10 % -2147483648 --> "; print 10 % -2147483648; print "\n";
print "2 % 2 --> "; print 2 % 2; print "\n";
print "2.5 % 5 --> "; print 2.5 % 5; print "\n";
print "5 % 2.0 --> "; print 5 % 2.0; print "\n";
print "5.0 % 2.0 --> "; print 5.0 % 2.0; print "\n";
print "\"5.5\" % 5 --> "; print "5.5" % 5; print "\n";
print "5 % \"5.5\" --> "; print 5 % "5.5"; print "\n";
print "5.5 % \"5\" --> "; print 5.5 % "5"; print "\n";
print "\"5.5\" % \"5\" --> "; print "5.5" % "5"; print "\n";
try {
  print "1 % 0 --> "; print 1 % 0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
try {
  print "1.0 % 0 --> "; print 1.0 % 0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
try {
  print "1 % 0.0 --> "; print 1 % 0.0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
try {
  print "1.0 % 0.0 --> "; print 1.0 % 0.0;
} catch (DivisionByZeroException $e) {
  print "Mod by zero\n";
}
print "\n";

print "5 & 3 --> "; print 5 & 3; print "\n";
print "5.0 & 3.0 --> "; print 5.0 & 3.0; print "\n";
print "\n";

print "5 | 3 --> "; print 5 | 3; print "\n";
print "5.0 | 3.0 --> "; print 5.0 | 3.0; print "\n";
print "\n";

print "5 ^ 3 --> "; print 5 ^ 3; print "\n";
print "5.0 ^ 3.0 --> "; print 5.0 ^ 3.0; print "\n";
print "\n";

print "5 << 1 --> "; print 5 << 1; print "\n";
print "5 << 1.0 --> "; print 5 << 1.0; print "\n";
print "5 << \"hi\" --> "; print 5 << "hi"; print "\n";
print "\n";

print "5 >> 1 --> "; print 5 >> 1; print "\n";
print "5 >> 1.0 --> "; print 5 >> 1.0; print "\n";
print "5 >> \"hi\" --> "; print 5 >> "hi"; print "\n";
print "\n";

print "!0 --> "; print !0; print "\n";
print "!5 --> "; print !5; print "\n";
print "!false --> "; print !false; print "\n";
print "!\"hi\" --> "; print !"hi"; print "\n";

print "3 === 4 --> "; print 3 === 4; print "\n";
print "3 === 3 --> "; print 3 === 3; print "\n";
print "4 === 3 --> "; print 4 === 3; print "\n";
print "\"4\" === 3 --> "; print "4" === 3; print "\n";
print "\n";

print "3 !== 4 --> "; print 3 !== 4; print "\n";
print "3 !== 3 --> "; print 3 !== 3; print "\n";
print "4 !== 3 --> "; print 4 !== 3; print "\n";
print "\"4\" !== 3 --> "; print "4" !== 3; print "\n";
print "\n";

print "3 == 4 --> "; print 3 == 4; print "\n";
print "3 == 3 --> "; print 3 == 3; print "\n";
print "4 == 3 --> "; print 4 == 3; print "\n";
print "\"4\" == 3 --> "; print "4" == 3; print "\n";
print "\n";

print "3 != 4 --> "; print 3 != 4; print "\n";
print "3 != 3 --> "; print 3 != 3; print "\n";
print "4 != 3 --> "; print 4 != 3; print "\n";
print "\"4\" != 3 --> "; print "4" != 3; print "\n";
print "\n";

print "3 < 4 --> "; print 3 < 4; print "\n";
print "3 < 3 --> "; print 3 < 3; print "\n";
print "4 < 3 --> "; print 4 < 3; print "\n";
print "\"4\" < 3 --> "; print "4" < 3; print "\n";
print "\n";

print "3 <= 4 --> "; print 3 <= 4; print "\n";
print "3 <= 3 --> "; print 3 <= 3; print "\n";
print "4 <= 3 --> "; print 4 <= 3; print "\n";
print "\"4\" <= 3 --> "; print "4" <= 3; print "\n";
print "\n";

if (true) {
  print "true\n";
} else {
  print "false\n";
}

if (true && false) {
  print "true && false\n";
} else {
  print "!(true && false)\n";
}

if (false || true) {
  print "false || true\n";
} else {
  print "!(false || true)\n";
}

print "3 && 4 --> "; print 3 && 4; print "\n";
print "3 && 3 --> "; print 3 && 3; print "\n";
print "4 && 3 --> "; print 4 && 3; print "\n";
print "\"4\" && 3 --> "; print "4" && 3; print "\n";
print "\n";

print "(string)42 --> "; print (string)42; print "\n";
print "(string)\"hi\" --> "; print (string)"hi"; print "\n";
print "\n";
}
