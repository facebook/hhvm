// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = .kind="class" .static {
// CHECK:   TMyShape: .public .type_constant *HackMixed
// CHECK: }

abstract class C {
  <<__Enforceable>>
  abstract const type TMyShape;

  // TEST-CHECK-BAL: define C$static.check2
  // CHECK: define C$static.check2($this: *C$static, $a: *HackMixed) : *HackBool {
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(102), $builtins.hack_string("root_name"), $builtins.hack_string("self"), $builtins.hack_string("access_list"), $builtins.hhbc_new_vec($builtins.hack_string("TMyShape")))
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n1, n0, $builtins.hack_int(1))
  // CHECK:   n3 = $builtins.hhbc_is_type_bool(n2)
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK:   ret n2
  // CHECK: }
  public static function check2(mixed $a): bool {
    return $a is self::TMyShape;
  }
}

// TEST-CHECK-BAL: type D$static
// CHECK: type D$static extends C$static = .kind="class" .static {
// CHECK:   TMyShape: .public .type_constant *HackMixed
// CHECK: }

class D extends C {
  const type TMyShape = shape(
    ?'a' => ?string,
    ?'b' => ?int,
  );

  // TEST-CHECK-BAL: define D$static.check3
  // CHECK: define D$static.check3($this: *D$static, $shape: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   ret null
  // CHECK: }
  public static function check3(self::TMyShape $shape)[]: void {
  }
}


// TEST-CHECK-BAL: define $root.check1
// CHECK: define $root.check1($this: *void, $a: *HackMixed) : *HackBool {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(102), $builtins.hack_string("root_name"), $builtins.hack_string("D"), $builtins.hack_string("access_list"), $builtins.hhbc_new_vec($builtins.hack_string("TMyShape")))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n1, n0, $builtins.hack_int(1))
// CHECK:   n3 = $builtins.hhbc_is_type_bool(n2)
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK:   ret n2
// CHECK: }
function check1(mixed $a): bool {
  return $a is D::TMyShape;
}


// TEST-CHECK-BAL: define $root.check2
// CHECK: define $root.check2($this: *void) : *HackMixed {
// CHECK: local $0: *void, $1: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict()
// CHECK:   n1 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(102), $builtins.hack_string("root_name"), $builtins.hack_string("D"), $builtins.hack_string("access_list"), $builtins.hhbc_new_vec($builtins.hack_string("TMyShape")))
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   store &$1 <- n1: *HackMixed
// CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n0, n1, $builtins.hack_int(1))
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK:   n3: *HackMixed = load &$0
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n3, $builtins.hack_bool(true))
// CHECK:   ret n3
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK:   n5: *HackMixed = load &$0
// CHECK:   n6: *HackMixed = load &$1
// CHECK:   n7 = $builtins.hhbc_throw_as_type_struct_exception(n5, n6)
// CHECK:   unreachable
// CHECK: }
function check2(): D::TMyShape {
  return dict[] as D::TMyShape;
}

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.check1(null, $builtins.hack_int(5))
// CHECK:   n1 = __sil_lazy_class_initialize(<D>)
// CHECK:   n2 = D$static.check2(n1, $builtins.hack_int(5))
// CHECK:   ret null
// CHECK: }
<<__EntryPoint>>
function main(): void {
  check1(5);
  D::check2(5);
}
