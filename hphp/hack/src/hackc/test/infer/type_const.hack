// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static extends HH::classname = .kind="class" .static .abstract {
// CHECK:   MyString: .public .constant .abstract *HackMixed;
// CHECK:   TMyShape: .public .type_constant *HackMixed
// CHECK: }

abstract class C {
  <<__Enforceable>>
  abstract const type TMyShape;
  abstract const string MyString;

  // TEST-CHECK-BAL: define C$static.check2
  // CHECK: define C$static.check2($this: .notnull *C$static, $a: *HackMixed) : .notnull *HackBool {
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(102), $builtins.hack_string("root_name"), $builtins.hack_string("self"), $builtins.hack_string("access_list"), $builtins.hhbc_new_vec($builtins.hack_string("TMyShape")))
  // CHECK: // .column 12
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK: // .column 12
  // CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n1, n0, $builtins.hack_int(1), $builtins.hack_int(0))
  // CHECK: // .column 5
  // CHECK:   n3 = $builtins.hhbc_is_type_bool(n2)
  // CHECK: // .column 5
  // CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
  // CHECK: // .column 5
  // CHECK:   ret n2
  // CHECK: }
  public static function check2(mixed $a): bool {
    return $a is self::TMyShape;
  }
}

// TEST-CHECK-BAL: type D$static
// CHECK: type D$static extends C$static = .kind="class" .static {
// CHECK:   MyString: .public .constant *HackString;
// CHECK:   TMyShape: .public .type_constant *HackMixed
// CHECK: }

class D extends C {
  const type TMyShape = shape(
    ?'a' => ?string,
    ?'b' => ?int,
  );
  const string MyString = "hello";

  // TEST-CHECK-BAL: define D$static.check3
  // CHECK: define D$static.check3($this: .notnull *D$static, $shape: .const_type="self::TMyShape" *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public static function check3(self::TMyShape $shape)[]: void {
  }
}


// TEST-CHECK-BAL: define $root.check1
// CHECK: define $root.check1($this: *void, $a: *HackMixed) : .notnull *HackBool {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(102), $builtins.hack_string("root_name"), $builtins.hack_string("D"), $builtins.hack_string("access_list"), $builtins.hhbc_new_vec($builtins.hack_string("TMyShape")))
// CHECK: // .column 10
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 10
// CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n1, n0, $builtins.hack_int(1), $builtins.hack_int(0))
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_is_type_bool(n2)
// CHECK: // .column 3
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n2, n3)
// CHECK: // .column 3
// CHECK:   ret n2
// CHECK: }
function check1(mixed $a): bool {
  return $a is D::TMyShape;
}


// TEST-CHECK-BAL: define $root.check2
// CHECK: define $root.check2($this: *void) : .const_type="D::TMyShape" *HackMixed {
// CHECK: local $0: *void, $1: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict()
// CHECK:   n1 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(102), $builtins.hack_string("root_name"), $builtins.hack_string("D"), $builtins.hack_string("access_list"), $builtins.hhbc_new_vec($builtins.hack_string("TMyShape")))
// CHECK: // .column 10
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK: // .column 10
// CHECK:   store &$1 <- n1: *HackMixed
// CHECK: // .column 10
// CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n0, n1, $builtins.hack_int(1), $builtins.hack_int(0))
// CHECK: // .column 10
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 10
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 10
// CHECK:   n3: *HackMixed = load &$0
// CHECK: // .column 10
// CHECK:   store &$1 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   n4 = $builtins.hhbc_verify_type_pred(n3, $builtins.hack_bool(true))
// CHECK: // .column 3
// CHECK:   ret n3
// CHECK: #b2:
// CHECK: // .column 10
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK: // .column 10
// CHECK:   n5: *HackMixed = load &$0
// CHECK: // .column 10
// CHECK:   n6: *HackMixed = load &$1
// CHECK: // .column 10
// CHECK:   n7 = $builtins.hhbc_throw_as_type_struct_exception(n5, n6)
// CHECK:   unreachable
// CHECK: }
function check2(): D::TMyShape {
  return dict[] as D::TMyShape;
}

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.check1(null, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   n1 = __sil_lazy_class_initialize(<D>)
// CHECK:   n2 = D$static.check2(n1, $builtins.hack_int(5))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
<<__EntryPoint>>
function main(): void {
  check1(5);
  D::check2(5);
}
