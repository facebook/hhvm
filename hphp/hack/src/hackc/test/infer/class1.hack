// RUN: %hackc compile-infer %s | FileCheck %s

class C {
  const A = 5;
}

class D extends C { }

class E extends D {
  public int $prop = C::A;
}

// TEST-CHECK-BAL: define C._86pinit
// CHECK: define C._86pinit($this: *C) : *HackMixed {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define D._86pinit
// CHECK: define D._86pinit($this: *D) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *D = load &$this
// CHECK:   n1 = C._86pinit(n0)
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define E._86pinit
// CHECK: define E._86pinit($this: *E$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *E = load &$this
// CHECK:   n1 = D._86pinit(n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true($builtins.hack_bool(false))
// CHECK:   jmp b3
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true($builtins.hack_bool(false))
// CHECK:   n2: *HackMixed = load &const::C$static::A
// CHECK:   n3 = &$this
// CHECK:   n4 = $builtins.hack_string("prop")
// CHECK:   n5 = $builtins.hack_dim_field_get(n3, n4)
// CHECK:   store n5 <- n2: *HackMixed
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   ret null
// CHECK: }
