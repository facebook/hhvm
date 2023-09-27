// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-IGN:
// CHECK: .source_language = "hack"

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("Hello, World!\n"))
// CHECK:   ret null
// CHECK: }
function main(): void {
  echo "Hello, World!\n";
}

// TEST-CHECK-BAL: define $root.escaped_string
// CHECK: define $root.escaped_string($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("This string has \042 a quote"))
// CHECK:   ret null
// CHECK: }
function escaped_string(): void {
  echo 'This string has " a quote';
}

// TEST-CHECK-BAL: define $root.cmp
// CHECK: define $root.cmp($this: *void, $a: *HackMixed, $b: *HackMixed) : *void {
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
// CHECK:   ret null
// CHECK: }
function cmp(mixed $a, mixed $b): void {
  if ($a == $b) {
    echo "equal";
  } else {
    echo "unequal";
  }
}

// TEST-CHECK-BAL: define $root.cmp2
// CHECK: define $root.cmp2($this: *void, $a: *HackInt, $b: *HackInt) : *void {
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
// CHECK:   ret null
// CHECK: }
function cmp2(int $a, int $b): void {
  if ($a == $b) {
    echo "equal";
  } else {
    echo "unequal";
  }
}

// TEST-CHECK-BAL: define $root.ret_str
// CHECK: define $root.ret_str($this: *void) : *HackString {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_is_type_str($builtins.hack_string("hello, world\n"))
// CHECK:   n1 = $builtins.hhbc_verify_type_pred($builtins.hack_string("hello, world\n"), n0)
// CHECK:   ret $builtins.hack_string("hello, world\n")
// CHECK: }
function ret_str(): string {
  return "hello, world\n";
}

// TEST-CHECK-BAL: define $root.bool_call
// CHECK: define $root.bool_call($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.f_bool(null, $builtins.hack_bool(false))
// CHECK:   n1 = $root.f_bool(null, $builtins.hack_bool(true))
// CHECK:   ret null
// CHECK: }
function bool_call(): void {
  f_bool(false);
  f_bool(true);
}

// TEST-CHECK-BAL: define $root.test_const
// CHECK: define $root.test_const($this: *void) : *void {
// CHECK: local $a: *void, $b: *void, $c: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec($builtins.hack_string("x"), $builtins.hack_float(2.0), $builtins.hack_int(3), $builtins.hack_bool(true))
// CHECK:   n1 = $builtins.hhbc_new_keyset_array($builtins.hack_string("xyzzy"), $builtins.hack_int(2))
// CHECK:   n2 = $builtins.hack_new_dict($builtins.hack_string("a"), $builtins.hack_string("b"), $builtins.hack_int(5), $builtins.hack_float(2.0))
// CHECK:   store &$a <- n0: *HackMixed
// CHECK:   store &$b <- n2: *HackMixed
// CHECK:   store &$c <- n1: *HackMixed
// CHECK:   ret null
// CHECK: }
function test_const(): void {
  $a = vec["x", 2.0, 3, true];
  $b = dict["a" => "b", 5 => 2.0];
  $c = keyset["xyzzy", 2];
}

// TEST-CHECK-BAL: define $root.float_arg
// CHECK: define $root.float_arg($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.f_float(null, $builtins.hack_float(3.14))
// CHECK:   n1 = $root.f_float(null, $builtins.hack_float(3.0))
// CHECK:   ret null
// CHECK: }
function float_arg(): void {
  f_float(3.14);
  f_float(3.0);
}

// TEST-CHECK-BAL: define $root.check_param_types
// CHECK: define $root.check_param_types($this: *void, $a: *HackInt, $b: *HackFloat, $c: *HackString) : *void {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }
function check_param_types(int $a, float $b, string $c): void {
}

// TEST-CHECK-BAL: define $root.check_is_class
// CHECK: define $root.check_is_class($this: *void, $a: *HackMixed) : *HackBool {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $builtins.hack_bool(__sil_instanceof(n0, <C>))
// CHECK:   n2 = $builtins.hhbc_is_type_bool(n1)
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK:   ret n1
// CHECK: }
function check_is_class(mixed $a): bool {
  return $a is C;
}

// TEST-CHECK-BAL: define $root.check_global
// CHECK: define $root.check_global($this: *void) : *void {
// CHECK: local $global_server: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &global::_SERVER
// CHECK:   store &$global_server <- n0: *HackMixed
// CHECK:   n1 = $root.sink(null, n0)
// CHECK:   ret null
// CHECK: }
function check_global(): void {
  $global_server = HH\global_get('_SERVER');
  sink($global_server);
}

// TEST-CHECK-BAL: define $root.check_constant
// CHECK: define $root.check_constant($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &gconst::GLOBAL_CONSTANT
// CHECK:   n1 = $builtins.hhbc_print(n0)
// CHECK:   ret null
// CHECK: }
function check_constant(): void {
  echo GLOBAL_CONSTANT;
}

// TEST-CHECK-BAL: define $root.check_file
// CHECK: define $root.check_file($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.printf(null, $builtins.hack_string("FILE: %s\n"), $builtins.hack_string("__FILE__"))
// CHECK:   ret null
// CHECK: }
function check_file(): void {
  printf("FILE: %s\n", __FILE__);
}

// TEST-CHECK-BAL: define $root.check_nothing
// CHECK: define $root.check_nothing($this: *void) : *noreturn {
// CHECK: #b0:
// CHECK:   n0 = $root.HH::invariant_violation(null, $builtins.hack_string("bad"))
// CHECK:   n1 = $builtins.hhbc_fatal($builtins.hack_string("invariant_violation"))
// CHECK:   unreachable
// CHECK: }
function check_nothing(): nothing {
  invariant(false, "bad");
}

// TEST-CHECK-BAL: define $root.check_noreturn
// CHECK: define $root.check_noreturn($this: *void) : *noreturn {
// CHECK: #b0:
// CHECK:   n0 = $root.HH::invariant_violation(null, $builtins.hack_string("bad"))
// CHECK:   n1 = $builtins.hhbc_fatal($builtins.hack_string("invariant_violation"))
// CHECK:   unreachable
// CHECK: }
function check_noreturn(): noreturn {
  invariant(false, "bad");
}

// TEST-CHECK-1: global global::_SERVER
// CHECK: global global::_SERVER : *HackMixed

// TEST-CHECK-1: global gconst::GLOBAL_CONSTANT
// CHECK: global gconst::GLOBAL_CONSTANT : *HackMixed
