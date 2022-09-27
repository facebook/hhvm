// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: define _Hfcall_func
// CHECK:   [[V:n[0-9]+]] = arg_pack_3(null, hack_int(1), hack_int(2), hack_int(3))
// CHECK:   _Hf([[V]])
function fcall_func(): void {
  f(1, 2, 3);
}

// CHECK: define _Hfcall_static
// CHECK:   [[STATIC:n[0-9]+]]: *void = load &static::_CC
// CHECK:   [[V:n[0-9]+]] = arg_pack_3([[STATIC]], hack_int(1), hack_int(2), hack_int(3))
// CHECK:   _MC::f([[V]])
function fcall_static(): void {
  C::f(1, 2, 3);
}
