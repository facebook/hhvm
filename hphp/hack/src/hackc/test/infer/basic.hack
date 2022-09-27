// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: attribute source_language = "hack"

// CHECK: define _Hmain(params: *HackParams) : *Mixed
// CHECK:  n0: *Mixed = load &params
// CHECK:  n1 = verify_param_count(n0, hack_int(0), hack_int(0))
// CHECK:  hhbc_print(hack_string("Hello, World!\n"))
// CHECK:  ret hack_null()
function main(): void {
  echo "Hello, World!\n";
}

// CHECK: define _Hcmp(params: *HackParams) : *Mixed
// CHECK: #b0:
// CHECK:   n0: *Mixed = load &params
// CHECK:   n1 = verify_param_count(n0, hack_int(2), hack_int(2))
// CHECK:   n2 = get_param(n0, hack_int(0))
// CHECK:   store &$a <- n2: *Mixed
// CHECK:   n3 = get_param(n0, hack_int(1))
// CHECK:   store &$b <- n3: *Mixed
// CHECK:   n4: *Mixed = load &$b
// CHECK:   n5: *Mixed = load &$a
// CHECK:   n6 = hhbc_cmp_eq(n5, n4)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! hack_is_true(n6)
// CHECK:   n7 = hhbc_print(hack_string("unequal"))
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune hack_is_true(n6)
// CHECK:   n8 = hhbc_print(hack_string("equal"))
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

// CHECK: define _Hret_str(params: *HackParams) : *Mixed {
// CHECK: #b0:
// CHECK:   n0: *Mixed = load &params
// CHECK:   n1 = verify_param_count(n0, hack_int(0), hack_int(0))
// CHECK:   n2 = hhbc_is_type_str(hack_string("hello, world\n"))
// CHECK:   n3 = hhbc_not(n2)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune hack_is_true(n3)
// CHECK:   n4 = hhbc_verify_failed()
// CHECK:   ret 0 // unreachable
// CHECK: #b2:
// CHECK:   prune ! hack_is_true(n3)
// CHECK:   ret hack_string("hello, world\n")
function ret_str(): string {
  return "hello, world\n";
}
