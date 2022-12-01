// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: define $root.fcall_func($this: *void) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $root.f(null, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   ret $builtins.hack_null()
function fcall_func(): void {
  f(1, 2, 3);
}

// CHECK: define $root.fcall_static($this: *void) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *C$static = load &C$static::static_singleton
// CHECK:   n1 = $builtins.lazy_initialize(n0)
// CHECK:   n2 = C$static.f(n0, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   ret $builtins.hack_null()
function fcall_static(): void {
  C::f(1, 2, 3);
}

// CHECK: define $root.fcall_method($this: *void, $a: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = n0.HackMixed.b($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   ret $builtins.hack_null()
function fcall_method(C $a): void {
  $a->b(1, 2, 3);
}

// CHECK: declare C$static.f(...): *HackMixed
