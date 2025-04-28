<?hh
// RUN: %hackc compile %s | FileCheck %s

// CHECK: .function {} (19,23) <"HH\\string" "HH\\string" > small() {
// CHECK:   .declvars $i $x;
// CHECK:   String "world"
// CHECK:   SetL $i
// CHECK:   PopC
// CHECK:   String "Hello "
// CHECK:   CGetL $i
// CHECK:   String "\n"
// CHECK:   ConcatN 3
// CHECK:   SetL $x
// CHECK:   PopC
// CHECK:   CGetL $x
// CHECK:   VerifyRetTypeC
// CHECK:   RetC
// CHECK: }
function small(): string {
  $i = "world";
  $x = "Hello $i\n"; // should concat
  return $x;
}

// CHECK: .function {} (49,55) <"HH\\string" "HH\\string" > large() {
// CHECK:   .declvars $a $b $c $d;
// CHECK:   String "Hello"
// CHECK:   SetL $a
// CHECK:   PopC
// CHECK:   String ", "
// CHECK:   SetL $b
// CHECK:   PopC
// CHECK:   String "world"
// CHECK:   SetL $c
// CHECK:   PopC
// CHECK:   String "!"
// CHECK:   SetL $d
// CHECK:   PopC
// CHECK:   CGetL $a
// CHECK:   CGetL $b
// CHECK:   CGetL $c
// CHECK:   CGetL $d
// CHECK:   ConcatN 4
// CHECK:   String "\n"
// CHECK:   Concat
// CHECK:   VerifyRetTypeC
// CHECK:   RetC
// CHECK: }
function large(): string {
  $a = "Hello";
  $b =  ", ";
  $c = "world";
  $d = "!";
  return "$a$b$c$d\n"; // should concatN
}

// CHECK: .function {} (86,92) <"HH\\string" "HH\\string" > massive() {
// CHECK:   .declvars $a $b $c $d;
// CHECK:   String "Hello"
// CHECK:   SetL $a
// CHECK:   PopC
// CHECK:   String ", "
// CHECK:   SetL $b
// CHECK:   PopC
// CHECK:   String "world"
// CHECK:   SetL $c
// CHECK:   PopC
// CHECK:   String "!"
// CHECK:   SetL $d
// CHECK:   PopC
// CHECK:   CGetL $a
// CHECK:   String " "
// CHECK:   CGetL $b
// CHECK:   String " "
// CHECK:   ConcatN 4
// CHECK:   CGetL $c
// CHECK:   String " "
// CHECK:   CGetL $d
// CHECK:   ConcatN 4
// CHECK:   CGetL $d
// CHECK:   String "\n"
// CHECK:   ConcatN 3
// CHECK:   VerifyRetTypeC
// CHECK:   RetC
// CHECK: }
function massive(): string {
  $a = "Hello";
  $b =  ", ";
  $c = "world";
  $d = "!";
  return "$a $b $c $d$d\n"; // should concatN
}

<<__EntryPoint>>
function main() : void {
  echo small();
  echo large();
}
