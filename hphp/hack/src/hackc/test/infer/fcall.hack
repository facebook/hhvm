// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast %s | FileCheck %s

class D extends B {
  // TEST-CHECK-BAL: define D.inst_fcall_self
  // CHECK: define D.inst_fcall_self($this: .notnull *D) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = D.bar(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_self(): void {
    self::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_self
  // CHECK: define D$static.static_fcall_self($this: .notnull *D$static) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = D$static.bar(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public static function static_fcall_self(): void {
    self::bar();
  }

  // TEST-CHECK-BAL: define D.inst_fcall_static
  // CHECK: define D.inst_fcall_static($this: .notnull *D) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = n0.?.bar()
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_static(): void {
    static::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_static
  // CHECK: define D$static.static_fcall_static($this: .notnull *D$static) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = n0.?.bar()
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public static function static_fcall_static(): void {
    static::bar();
  }

  // TEST-CHECK-BAL: define D.inst_fcall_parent
  // CHECK: define D.inst_fcall_parent($this: .notnull *D) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *D = load &$this
  // CHECK:   n1 = B.bar(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_parent(): void {
    parent::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_parent
  // CHECK: define D$static.static_fcall_parent($this: .notnull *D$static) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = B$static.bar(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public static function static_fcall_parent(): void {
    parent::bar();
  }

  public static function bar(): void { }
}

// TEST-CHECK-BAL: define $root.fcall_func
// CHECK: define $root.fcall_func($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = $root.f(null, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }

function fcall_func(): void {
  f(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_static
// CHECK: define $root.fcall_static($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0 = __sil_lazy_class_initialize(<C>)
// CHECK:   n1 = C$static.f(n0, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_static(): void {
  C::f(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_method
// CHECK: define $root.fcall_method($this: *void, $a: *C) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 3
// CHECK:   n1 = n0.?.b($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_method(C $a): void {
  $a->b(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_nullsafe
// CHECK: define $root.fcall_nullsafe($this: *void, $c: *C) : *void {
// CHECK: #b0:
// CHECK: // .column 5
// CHECK:   n0: *HackMixed = load &$c
// CHECK: // .column 5
// CHECK:   n1 = $builtins.hhbc_is_type_null(n0)
// CHECK: // .column 5
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 5
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK: // .column 5
// CHECK:   jmp b3(null)
// CHECK: #b2:
// CHECK: // .column 5
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK: // .column 5
// CHECK:   n2 = n0.?.g()
// CHECK: // .column 5
// CHECK:   jmp b3(n2)
// CHECK: #b3(n3: *HackMixed):
// CHECK: // .column 3
// CHECK:   n4 = $root.f(null, n3)
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_nullsafe(?C $c): void {
  f($c?->g());
}

// TEST-CHECK-BAL: define $root.fcall_class_meth
// CHECK: define $root.fcall_class_meth($this: *void) : *void {
// CHECK: local $x: *void, $0: *void
// CHECK: #b0:
// CHECK: // .column 8
// CHECK:   n0 = __sil_lazy_class_initialize(<C>)
// CHECK:   n1 = __sil_allocate(<C$static_sb$curry>)
// CHECK:   store n1.C$static_sb$curry.this <- n0: *C$static
// CHECK: // .column 3
// CHECK:   store &$x <- n1: *HackMixed
// CHECK: // .column 3
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 3
// CHECK:   n2: *HackMixed = load &$x
// CHECK: // .column 3
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK: // .column 3
// CHECK:   n3: *HackMixed = load &$0
// CHECK: // .column 3
// CHECK:   n4 = n3.?.__invoke($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n5: *HackMixed):
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   throw n5
// CHECK: #b3:
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_class_meth(): void {
  $x = C::sb<>;
  $x(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_func_invoke
// CHECK: define $root.fcall_func_invoke($this: *void) : *void {
// CHECK: local $x: *void, $0: *void
// CHECK: #b0:
// CHECK: // .column 8
// CHECK:   n0 = __sil_allocate(<f$curry>)
// CHECK: // .column 3
// CHECK:   store &$x <- n0: *HackMixed
// CHECK: // .column 3
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 3
// CHECK:   n1: *HackMixed = load &$x
// CHECK: // .column 3
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 3
// CHECK:   n2: *HackMixed = load &$0
// CHECK: // .column 3
// CHECK:   n3 = n2.?.__invoke($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n4: *HackMixed):
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   throw n4
// CHECK: #b3:
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_func_invoke(): void {
  $x = f<>;
  $x(1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_splat
// CHECK: define $root.fcall_splat($this: *void) : *void {
// CHECK: local $x: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec($builtins.hack_int(2), $builtins.hack_int(3), $builtins.hack_int(4))
// CHECK: // .column 3
// CHECK:   store &$x <- n0: *HackMixed
// CHECK: // .column 11
// CHECK:   n1: *HackMixed = load &$x
// CHECK: // .column 3
// CHECK:   n2 = $builtins.__sil_splat(n1)
// CHECK:   n3 = $root.f(null, $builtins.hack_int(1), n2)
// CHECK: // .column 2
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
// CHECK: // .column 8
// CHECK:   n0 = $root.HH::meth_caller(null, __sil_get_lazy_class(<C>), $builtins.hack_string("b"))
// CHECK: // .column 3
// CHECK:   store &$x <- n0: *HackMixed
// CHECK: // .column 3
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 3
// CHECK:   n1: *HackMixed = load &$x
// CHECK: // .column 3
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 6
// CHECK:   n2: *HackMixed = load &$b
// CHECK: // .column 3
// CHECK:   n3: *HackMixed = load &$0
// CHECK: // .column 3
// CHECK:   n4 = n3.?.__invoke(n2, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n5: *HackMixed):
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   throw n5
// CHECK: #b3:
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_meth_caller(C $b): void {
  $x = meth_caller(C::class, 'b');
  $x($b, 1, 2, 3);
}

// TEST-CHECK-BAL: define $root.fcall_cls_method
// CHECK: define $root.fcall_cls_method($this: *void, $a: *HH::classname) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("HH\\classname"))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$a
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 3
// CHECK:   n3: *HackMixed = load &$a
// CHECK: // .column 3
// CHECK:   n4 = n3.?.static_fcall_self()
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_cls_method(classname<D> $a): void {
  $a::static_fcall_self();
}

// TEST-CHECK-BAL: define $root.fcall_readonly
// CHECK: define $root.fcall_readonly($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 18
// CHECK:   n0 = $root.g(null)
// CHECK: // .column 3
// CHECK:   n1 = $root.readonly_param(null, $builtins.__sil_readonly(n0))
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function fcall_readonly(): void {
  readonly_param(readonly g());
}

// TEST-CHECK-BAL: type f$curry
// CHECK: type f$curry = .kind="class" .final {
// CHECK: }

// TEST-CHECK-BAL: define .final .curry f$curry.__invoke
// CHECK: define .final .curry f$curry.__invoke(this: *f$curry, args: .variadic *HackVec) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *f$curry = load &this
// CHECK:   n1: *HackVec = load &args
// CHECK:   n2 = $builtins.__sil_splat(n1)
// CHECK:   n3 = $root.f(null, n2)
// CHECK:   ret n3
// CHECK: }

// TEST-CHECK-BAL: type C$static_sb$curry
// CHECK: type C$static_sb$curry = .kind="class" .final {
// CHECK:   this: .private *C$static
// CHECK: }

// TEST-CHECK-BAL: define .final .curry C$static_sb$curry.__invoke
// CHECK: define .final .curry C$static_sb$curry.__invoke(this: *C$static_sb$curry, args: .variadic *HackVec) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *C$static_sb$curry = load &this
// CHECK:   n1: *HackVec = load &args
// CHECK:   n2 = $builtins.__sil_splat(n1)
// CHECK:   n3: *C$static = load n0.C$static_sb$curry.this
// CHECK:   n4 = n3.C$static.sb(n2)
// CHECK:   ret n4
// CHECK: }

// TEST-CHECK-1: declare C$static.f
// CHECK: declare C$static.f(...): *HackMixed
