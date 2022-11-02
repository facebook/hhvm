// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: .source_language = "hack"

// CHECK: define $root.no_locals(this: *void, $a: *HackInt) : *void {
// CHECK: #b0:
// CHECK: } 
function no_locals(int $a) : void {
}

// CHECK: define $root.only_locals(this: *void) : *void {
// CHECK: local $a: *void, $b: *void
// CHECK: #b0:
// CHECK: } 
function only_locals() : void {
  $a = 1;
  $b = 2;
}

// CHECK: define $root.params_and_locals(this: *void, $a: *HackInt) : *void {
// CHECK: local $b: *void, $c: *void
// CHECK: #b0:
// CHECK: } 
function params_and_locals(int $a) : void {
  $b = 1;
  $c = 2;
}
