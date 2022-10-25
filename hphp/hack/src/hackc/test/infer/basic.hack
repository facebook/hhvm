// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: attribute source_language = "hack"

// CHECK: define _Hmain(this: *void) : *void {
// CHECK:  n0 = hhbc_print(hack_string("Hello, World!\n"))
// CHECK:  ret hack_null()
function main(): void {
  echo "Hello, World!\n";
}

// CHECK: define _Hcmp(this: *void, $a: *Mixed, $b: *Mixed) : *void {
// CHECK: #b0:
// CHECK:   n0: *Mixed = load &$b
// CHECK:   n1: *Mixed = load &$a
// CHECK:   n2 = hhbc_cmp_eq(n1, n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! hack_is_true(n2)
// CHECK:   n3 = hhbc_print(hack_string("unequal"))
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune hack_is_true(n2)
// CHECK:   n4 = hhbc_print(hack_string("equal"))
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   ret hack_null()
function cmp(mixed $a, mixed $b): void {
  if ($a == $b) {
    echo "equal";
  } else {
    echo "unequal";
  }
}

// CHECK: define _Hret_str(this: *void) : *string {
// CHECK: #b0:
// CHECK:   n0 = hhbc_is_type_str(hack_string("hello, world\n"))
// CHECK:   n1 = hhbc_not(n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune hack_is_true(n1)
// CHECK:   n2 = hhbc_verify_failed()
// CHECK:   ret 0 // unreachable
// CHECK: #b2:
// CHECK:   prune ! hack_is_true(n1)
// CHECK:   ret hack_string("hello, world\n")
function ret_str(): string {
  return "hello, world\n";
}

// CHECK: define _Hbool_call(this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = _Hf_bool(null, hack_bool(false))
// CHECK:   n1 = _Hf_bool(null, hack_bool(true))
// CHECK:   ret hack_null()
function bool_call(): void {
  f_bool(false);
  f_bool(true);
}
