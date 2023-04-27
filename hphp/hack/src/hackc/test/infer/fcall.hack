// RUN: %hackc compile-infer %s | FileCheck %s

class D extends B {
  // TEST-CHECK-BAL: define D.inst_fcall_self
  // CHECK: define D.inst_fcall_self($this: *D) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = D.bar(n0)
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_self(): void {
    self::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_self
  // CHECK: define D$static.static_fcall_self($this: *D$static) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = D$static.bar(n0)
  // CHECK:   ret null
  // CHECK: }
  public static function static_fcall_self(): void {
    self::bar();
  }

  // TEST-CHECK-BAL: define D.inst_fcall_static
  // CHECK: define D.inst_fcall_static($this: *D) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = n0.D.bar()
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_static(): void {
    static::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_static
  // CHECK: define D$static.static_fcall_static($this: *D$static) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = n0.D$static.bar()
  // CHECK:   ret null
  // CHECK: }
  public static function static_fcall_static(): void {
    static::bar();
  }

  // TEST-CHECK-BAL: define D.inst_fcall_parent
  // CHECK: define D.inst_fcall_parent($this: *D) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = B.bar(n0)
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_parent(): void {
    parent::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_parent
  // CHECK: define D$static.static_fcall_parent($this: *D$static) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = B$static.bar(n0)
  // CHECK:   ret null
  // CHECK: }
  public static function static_fcall_parent(): void {
    parent::bar();
  }

  public static function bar(): void { }
}

// TEST-CHECK-1: define $root.MethCaller$C$b
// CHECK: define $root.MethCaller$C$b($this: *void, $o: *HackMixed, $args: *array) : *HackMixed {

// TEST-CHECK-BAL: define $root.fcall_func
// CHECK: define $root.fcall_func($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.f(null, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   ret null
// CHECK: }
function fcall_func(): void {
  f(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_static
// CHECK: define $root.fcall_static($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = __sil_lazy_class_initialize(<C>)
// CHECK:   n1 = C$static.f(n0, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   ret null
// CHECK: }
function fcall_static(): void {
  C::f(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_method
// CHECK: define $root.fcall_method($this: *void, $a: *C) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = n0.HackMixed.b($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   ret null
// CHECK: }
function fcall_method(C $a): void {
  $a->b(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_nullsafe
// CHECK: define $root.fcall_nullsafe($this: *void, $c: *C) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$c
// CHECK:   n1 = $builtins.hhbc_is_type_null(n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   jmp b3(null)
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   n2 = n0.HackMixed.g()
// CHECK:   jmp b3(n2)
// CHECK: #b3(n3: *HackMixed):
// CHECK:   n4 = $root.f(null, n3)
// CHECK:   ret null
// CHECK: }
function fcall_nullsafe(?C $c): void {
  f($c?->g());
}

// TEST-CHECK-BAL: define $root.fcall_class_meth
// CHECK: define $root.fcall_class_meth($this: *void) : *void {
// CHECK: local $x: *void, $0: *void
// CHECK: #b0:
// CHECK:   n0 = __sil_lazy_class_initialize(<C>)
// CHECK:   n1 = __sil_allocate_curry("<C$static>", "sb", n0)
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n2: *HackMixed = load &$x
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK:   n3: *HackMixed = load &$0
// CHECK:   n4 = n3.HackMixed.__invoke($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n5: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   n6 = $builtins.hhbc_throw(n5)
// CHECK:   unreachable
// CHECK: #b3:
// CHECK:   ret null
// CHECK: }
function fcall_class_meth(): void {
  $x = C::sb<>;
  $x(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_splat
// CHECK: define $root.fcall_splat($this: *void) : *void {
// CHECK: local $x: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec($builtins.hack_int(2), $builtins.hack_int(3), $builtins.hack_int(4))
// CHECK:   store &$x <- n0: *HackMixed
// CHECK:   n1: *HackMixed = load &$x
// CHECK:   n2 = $builtins.__sil_splat(n1)
// CHECK:   n3 = $root.f(null, $builtins.hack_int(1), n2)
// CHECK:   ret null
// CHECK: }
function fcall_splat(): void {
  $x = vec[2, 3, 4];
  f(1, ...$x);
}

// TEST-CHECK-BAL: define $root.fcall_meth_caller
// CHECK: define $root.fcall_meth_caller($this: *void, $b: *C) : *void {
// CHECK: local $x: *void, $0: *void
// CHECK: #b0:
// CHECK:   n0 = __sil_allocate_curry("<$root>", "MethCaller$C$b", null)
// CHECK:   store &$x <- n0: *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *HackMixed = load &$x
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3: *HackMixed = load &$0
// CHECK:   n4 = n3.HackMixed.__invoke(n2, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n5: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   n6 = $builtins.hhbc_throw(n5)
// CHECK:   unreachable
// CHECK: #b3:
// CHECK:   ret null
// CHECK: }
function fcall_meth_caller(C $b): void {
  $x = meth_caller(C::class, 'b');
  $x($b, 1, 2, 3);
}

// TEST-CHECK-1: declare C$static.f
// CHECK: declare C$static.f(...): *HackMixed

// TEST-CHECK-1: declare __sil_allocate_curry
// CHECK: declare __sil_allocate_curry(...): *HackMixed
