// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: define _Hfcall_func
// CHECK: #b0:
// CHECK:   n0: *Mixed = load &params
// CHECK:   n1 = verify_param_count(n0, hack_int(0), hack_int(0))
// CHECK:   n2 = arg_pack_3(null, hack_int(1), hack_int(2), hack_int(3))
// CHECK:   n3 = _Hf(n2)
function fcall_func(): void {
  f(1, 2, 3);
}

// CHECK: define _Hfcall_static
// CHECK: #b0:
// CHECK:   n0: *Mixed = load &params
// CHECK:   n1 = verify_param_count(n0, hack_int(0), hack_int(0))
// CHECK:   n2 = get_static::_CC()
// CHECK:   n3 = arg_pack_3(n2, hack_int(1), hack_int(2), hack_int(3))
// CHECK:   n4 = _MC::f(n3)
function fcall_static(): void {
  C::f(1, 2, 3);
}
