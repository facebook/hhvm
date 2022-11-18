// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: .source_language = "hack"

// CHECK: define $root.main(this: *void) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("Hello, World!\n")
// CHECK:   n1 = $builtins.hhbc_print(n0)
// CHECK:   ret $builtins.hack_null()
function main(): void {
  echo "Hello, World!\n";
}

// CHECK: define $root.escaped_string(this: *void) : *HackMixed {
// CHECK: local base: *HackMixed
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("This string has \042 a quote")
// CHECK:   n1 = $builtins.hhbc_print(n0)
// CHECK:   ret $builtins.hack_null()
// CHECK: }
function escaped_string(): void {
  echo 'This string has " a quote';
}

// CHECK: define $root.cmp(this: *void, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("equal")
// CHECK:   n1 = $builtins.hack_string("unequal")
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3: *HackMixed = load &$a
// CHECK:   n4 = $builtins.hhbc_cmp_eq(n3, n2)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n5 = $builtins.hhbc_print(n1)
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   n6 = $builtins.hhbc_print(n0)
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

// CHECK: define $root.cmp2(this: *void, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("equal")
// CHECK:   n1 = $builtins.hack_string("unequal")
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3: *HackMixed = load &$a
// CHECK:   n4 = $builtins.hhbc_cmp_eq(n3, n2)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   n5 = $builtins.hhbc_print(n1)
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   n6 = $builtins.hhbc_print(n0)
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   ret $builtins.hack_null()
function cmp2(int $a, int $b): void {
  if ($a == $b) {
    echo "equal";
  } else {
    echo "unequal";
  }
}

// CHECK: define $root.ret_str(this: *void) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("hello, world\n")
// CHECK:   n1 = $builtins.hhbc_is_type_str(n0)
// CHECK:   n2 = $builtins.hhbc_verify_type_pred(n0, n1)
// CHECK:   ret n0
function ret_str(): string {
  return "hello, world\n";
}

// CHECK: define $root.bool_call(this: *void) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $root.f_bool(null, $builtins.hack_bool(false))
// CHECK:   n1 = $root.f_bool(null, $builtins.hack_bool(true))
// CHECK:   ret $builtins.hack_null()
function bool_call(): void {
  f_bool(false);
  f_bool(true);
}

// CHECK: define $root.test_const(this: *void) : *HackMixed {
// CHECK: local $a: *void, $b: *void, $c: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec($builtins.hack_string("x"), $builtins.hack_float(2.0), $builtins.hack_int(3), $builtins.hack_bool(true))
// CHECK:   n1 = $builtins.hhbc_new_keyset_array($builtins.hack_string("xyzzy"), $builtins.hack_int(2))
// CHECK:   n2 = $builtins.hack_new_dict($builtins.hack_string("a"), $builtins.hack_string("b"), $builtins.hack_int(5), $builtins.hack_float(2.0))
// CHECK:   store &$a <- n0: *HackMixed
// CHECK:   store &$b <- n2: *HackMixed
// CHECK:   store &$c <- n1: *HackMixed
// CHECK:   ret $builtins.hack_null()
// CHECK: }
function test_const(): void {
  $a = vec["x", 2.0, 3, true];
  $b = dict["a" => "b", 5 => 2.0];
  $c = keyset["xyzzy", 2];
}

// CHECK: define $root.float_arg(this: *void) : *HackMixed {
// CHECK: local base: *HackMixed
// CHECK: #b0:
// CHECK:   n0 = $root.f_float(null, $builtins.hack_float(3.14))
// CHECK:   n1 = $root.f_float(null, $builtins.hack_float(3.0))
// CHECK:   ret $builtins.hack_null()
// CHECK: }
function float_arg(): void {
  f_float(3.14);
  f_float(3.0);
}
