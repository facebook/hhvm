// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-IGN:
// CHECK: .source_language = "hack"

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("Hello, World!\n"))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function main(): void {
  echo "Hello, World!\n";
}

// TEST-CHECK-BAL: define $root.escaped_string
// CHECK: define $root.escaped_string($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("This string has \042 a quote"))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function escaped_string(): void {
  echo 'This string has " a quote';
}

// TEST-CHECK-BAL: define $root.cmp
// CHECK: define $root.cmp($this: *void, $a: *HackMixed, $b: *HackMixed) : *void {
// CHECK: #b0:
// CHECK: // .column 13
// CHECK:   n0: *HackMixed = load &$b
// CHECK: // .column 13
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 13
// CHECK:   n2 = $builtins.hhbc_cmp_eq(n1, n0)
// CHECK: // .column 13
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 13
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK: // .column 5
// CHECK:   n3 = $builtins.hhbc_print($builtins.hack_string("unequal"))
// CHECK: // .column 5
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK: // .column 13
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 5
// CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("equal"))
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK: // .column 2
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
// CHECK: define $root.cmp2($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : *void {
// CHECK: #b0:
// CHECK: // .column 13
// CHECK:   n0: *HackMixed = load &$b
// CHECK: // .column 13
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 13
// CHECK:   n2 = $builtins.hhbc_cmp_eq(n1, n0)
// CHECK: // .column 13
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 13
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK: // .column 5
// CHECK:   n3 = $builtins.hhbc_print($builtins.hack_string("unequal"))
// CHECK: // .column 5
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK: // .column 13
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 5
// CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("equal"))
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK: // .column 2
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
// CHECK: define $root.ret_str($this: *void) : .notnull *HackString {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $builtins.hhbc_is_type_str($builtins.hack_string("hello, world\n"))
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_verify_type_pred($builtins.hack_string("hello, world\n"), n0)
// CHECK: // .column 3
// CHECK:   ret $builtins.hack_string("hello, world\n")
// CHECK: }
function ret_str(): string {
  return "hello, world\n";
}

// TEST-CHECK-BAL: define $root.bool_call
// CHECK: define $root.bool_call($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.f_bool(null, $builtins.hack_bool(false))
// CHECK: // .column 3
// CHECK:   n1 = $root.f_bool(null, $builtins.hack_bool(true))
// CHECK: // .column 2
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
// CHECK: // .column 3
// CHECK:   store &$a <- n0: *HackMixed
// CHECK: // .column 3
// CHECK:   store &$b <- n2: *HackMixed
// CHECK: // .column 3
// CHECK:   store &$c <- n1: *HackMixed
// CHECK: // .column 2
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
// CHECK: // .column 3
// CHECK:   n0 = $root.f_float(null, $builtins.hack_float(3.14))
// CHECK: // .column 3
// CHECK:   n1 = $root.f_float(null, $builtins.hack_float(3.0))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function float_arg(): void {
  f_float(3.14);
  f_float(3.0);
}

// TEST-CHECK-BAL: define $root.check_param_types
// CHECK: define $root.check_param_types($this: *void, $a: .notnull *HackInt, $b: .notnull *HackFloat, $c: .notnull *HackString) : *void {
// CHECK: #b0:
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function check_param_types(int $a, float $b, string $c): void {
}

// TEST-CHECK-BAL: define $root.check_is_class
// CHECK: define $root.check_is_class($this: *void, $a: *HackMixed) : .notnull *HackBool {
// CHECK: #b0:
// CHECK: // .column 10
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 10
// CHECK:   n1 = $builtins.hack_bool(__sil_instanceof(n0, <C>, 0))
// CHECK: // .column 3
// CHECK:   n2 = $builtins.hhbc_is_type_bool(n1)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK: // .column 3
// CHECK:   ret n1
// CHECK: }
function check_is_class(mixed $a): bool {
  return $a is C;
}

// TEST-CHECK-BAL: define $root.check_is_class_nullable
// CHECK: define $root.check_is_class_nullable($this: *void, $a: *HackMixed) : .notnull *HackBool {
// CHECK: #b0:
// CHECK: // .column 10
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 10
// CHECK:   n1 = $builtins.hack_bool(__sil_instanceof(n0, <C>, 1))
// CHECK: // .column 3
// CHECK:   n2 = $builtins.hhbc_is_type_bool(n1)
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_verify_type_pred(n1, n2)
// CHECK: // .column 3
// CHECK:   ret n1
// CHECK: }
function check_is_class_nullable(mixed $a): bool {
  return $a is ?C;
}

// TEST-CHECK-BAL: define $root.check_global
// CHECK: define $root.check_global($this: *void) : *void {
// CHECK: local $global_server: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &global::_SERVER
// CHECK: // .column 3
// CHECK:   store &$global_server <- n0: *HackMixed
// CHECK: // .column 8
// CHECK:   n1: *HackMixed = load &$global_server
// CHECK: // .column 3
// CHECK:   n2 = $root.sink(null, n1)
// CHECK: // .column 2
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
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_print(n0)
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function check_constant(): void {
  echo GLOBAL_CONSTANT;
}

// TEST-CHECK-BAL: define $root.check_file
// CHECK: define $root.check_file($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.printf(null, $builtins.hack_string("FILE: %s\n"), $builtins.hack_string("__FILE__"))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function check_file(): void {
  printf("FILE: %s\n", __FILE__);
}

// TEST-CHECK-BAL: define $root.check_nothing
// CHECK: define $root.check_nothing($this: *void) : *noreturn {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.HH::invariant_violation(null, $builtins.hack_string("bad"))
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_fatal($builtins.hack_string("invariant_violation"))
// CHECK:   unreachable
// CHECK: }
function check_nothing(): nothing {
  invariant(false, "bad");
}


// TEST-CHECK-BAL: define $root.check_nullable_types
// CHECK: define $root.check_nullable_types($this: *void, $i: .notnull *HackInt, $j: *HackInt, $f: .notnull *HackFloat, $g: *HackFloat, $u: .notnull *HackString, $v: *HackString) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("i: %d, j: %d, f: %f, g: %f, u: %s, v: %s\n")
// CHECK: // .column 56
// CHECK:   n1: *HackMixed = load &$i
// CHECK: // .column 60
// CHECK:   n2: *HackMixed = load &$j
// CHECK: // .column 64
// CHECK:   n3: *HackMixed = load &$f
// CHECK: // .column 68
// CHECK:   n4: *HackMixed = load &$g
// CHECK: // .column 72
// CHECK:   n5: *HackMixed = load &$u
// CHECK: // .column 76
// CHECK:   n6: *HackMixed = load &$v
// CHECK: // .column 3
// CHECK:   n7 = $root.printf(null, n0, n1, n2, n3, n4, n5, n6)
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function check_nullable_types(int $i, ?int $j, float $f, ?float $g, string $u, ?string $v): void {
  printf("i: %d, j: %d, f: %f, g: %f, u: %s, v: %s\n", $i, $j, $f, $g, $u, $v);
}



// TEST-CHECK-BAL: define $root.check_noreturn
// CHECK: define $root.check_noreturn($this: *void) : *noreturn {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.HH::invariant_violation(null, $builtins.hack_string("bad"))
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_fatal($builtins.hack_string("invariant_violation"))
// CHECK:   unreachable
// CHECK: }
function check_noreturn(): noreturn {
  invariant(false, "bad");
}

// TEST-CHECK-1: global gconst::GLOBAL_CONSTANT
// CHECK: global gconst::GLOBAL_CONSTANT : *HackMixed

// TEST-CHECK-1: global global::_SERVER
// CHECK: global global::_SERVER : *HackMixed
