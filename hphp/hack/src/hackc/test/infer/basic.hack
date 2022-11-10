// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: .source_language = "hack"

// CHECK: define $root.main(this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("Hello, World!\n"))
// CHECK:   ret $builtins.hack_null()
function main(): void {
  echo "Hello, World!\n";
}

// CHECK: define $root.cmp(this: *void, $a: *HackMixed, $b: *HackMixed) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$b
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_cmp_eq(n1, n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK:   n3 = $builtins.hhbc_print($builtins.hack_string("unequal"))
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("equal"))
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   ret $builtins.hack_null()
function cmp(mixed $a, mixed $b): void {
  if ($a == $b) {
    echo "equal";
  } else {
    echo "unequal";
  }
}

// CHECK: define $root.cmp2(this: *void, $a: *HackInt, $b: *HackInt) : *void {
function cmp2(int $a, int $b): void {
  if ($a == $b) {
    echo "equal";
  } else {
    echo "unequal";
  }
}

// CHECK: define $root.ret_str(this: *void) : *HackString {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_is_type_str($builtins.hack_string("hello, world\n"))
// CHECK:   n1 = $builtins.hhbc_not(n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   n2 = $builtins.hhbc_verify_failed()
// CHECK:   unreachable
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   ret $builtins.hack_string("hello, world\n")
function ret_str(): string {
  return "hello, world\n";
}

// CHECK: define $root.bool_call(this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.f_bool(null, $builtins.hack_bool(false))
// CHECK:   n1 = $root.f_bool(null, $builtins.hack_bool(true))
// CHECK:   ret $builtins.hack_null()
function bool_call(): void {
  f_bool(false);
  f_bool(true);
}

// CHECK: declare $builtins.hack_is_true(*HackMixed): int
