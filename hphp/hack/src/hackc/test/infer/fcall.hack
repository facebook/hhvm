// RUN: %hackc compile-infer %s | FileCheck %s

class D extends B {
  // CHECK: define D.inst_fcall_self($this: *D) : *HackMixed {
  // ...
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = D.bar(n0)
  // ...
  // CHECK: }
  public function inst_fcall_self(): void {
    self::bar();
  }

  // CHECK: define D$static.static_fcall_self($this: *D$static) : *HackMixed {
  // ...
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = D$static.bar(n0)
  // ...
  // CHECK: }
  public static function static_fcall_self(): void {
    self::bar();
  }

  // CHECK: define D.inst_fcall_static($this: *D) : *HackMixed {
  // ...
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = n0.D.bar()
  // ...
  // CHECK: }
  public function inst_fcall_static(): void {
    static::bar();
  }

  // CHECK: define D$static.static_fcall_static($this: *D$static) : *HackMixed {
  // ...
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = n0.D$static.bar()
  // ...
  // CHECK: }
  public static function static_fcall_static(): void {
    static::bar();
  }

  // CHECK: define D.inst_fcall_parent($this: *D) : *HackMixed {
  // ...
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = B.bar(n0)
  // ...
  // CHECK: }
  public function inst_fcall_parent(): void {
    parent::bar();
  }

  // CHECK: define D$static.static_fcall_parent($this: *D$static) : *HackMixed {
  // ...
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = B$static.bar(n0)
  // ...
  // CHECK: }
  public static function static_fcall_parent(): void {
    parent::bar();
  }

  public static function bar(): void { }
}

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
