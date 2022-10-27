// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: define $root.fcall_func(this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.f(null, hack_int(1), hack_int(2), hack_int(3))
// CHECK:   ret hack_null()
function fcall_func(): void {
  f(1, 2, 3);
}

// CHECK: define $root.fcall_static(this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0: *C$static = load &static_singleton::C
// CHECK:   n1 = __sil_lazy_initialize(n0)
// CHECK:   n2 = C.f(n0, hack_int(1), hack_int(2), hack_int(3))
// CHECK:   ret hack_null()
function fcall_static(): void {
  C::f(1, 2, 3);
}
