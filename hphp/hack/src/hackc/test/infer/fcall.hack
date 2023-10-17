// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

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
  // CHECK:   n1 = n0.?.bar()
  // CHECK:   ret null
  // CHECK: }
  public function inst_fcall_static(): void {
    static::bar();
  }

  // TEST-CHECK-BAL: define D$static.static_fcall_static
  // CHECK: define D$static.static_fcall_static($this: *D$static) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *D$static = load &$this
  // CHECK:   n1 = n0.?.bar()
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
// CHECK:   n1 = n0.?.b($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
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
// CHECK:   n2 = n0.?.g()
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
// CHECK:   n1 = __sil_allocate(<C$static_sb$curry>)
// CHECK:   store n1.C$static_sb$curry.this <- n0: *C$static
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n2: *HackMixed = load &$x
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK:   n3 = n2.?.__invoke($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n4: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   n5 = $builtins.hhbc_throw(n4)
// CHECK:   unreachable
// CHECK: #b3:
// CHECK:   store &$0 <- null: *HackMixed
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
// CHECK:   n0 = __sil_allocate(<f$curry>)
// CHECK:   store &$x <- n0: *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *HackMixed = load &$x
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2 = n1.?.__invoke($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n3: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   n4 = $builtins.hhbc_throw(n3)
// CHECK:   unreachable
// CHECK: #b3:
// CHECK:   store &$0 <- null: *HackMixed
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
// CHECK:   store &$x <- n0: *HackMixed
// CHECK:   n1 = $builtins.__sil_splat(n0)
// CHECK:   n2 = $root.f(null, $builtins.hack_int(1), n1)
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
// CHECK:   n0 = __sil_allocate(<MethCaller$C$b$curry>)
// CHECK:   store n0.MethCaller$C$b$curry.arg0 <- null: *void
// CHECK:   store &$x <- n0: *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *HackMixed = load &$x
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3 = n1.?.__invoke(n2, $builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
// CHECK:   jmp b3
// CHECK:   .handlers b2
// CHECK: #b2(n4: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   n5 = $builtins.hhbc_throw(n4)
// CHECK:   unreachable
// CHECK: #b3:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   ret null
// CHECK: }
function fcall_meth_caller(C $b): void {
  $x = meth_caller(C::class, 'b');
  $x($b, 1, 2, 3);
}

// TEST-CHECK-1: define $root.MethCaller$C$b
// CHECK: define $root.MethCaller$C$b($this: *void, $o: *HackMixed, $args: .variadic .typevar="array" *HackMixed) : *HackMixed {

// TEST-CHECK-BAL: define $root.fcall_cls_method
// CHECK: define $root.fcall_cls_method($this: *void, $a: *Classname) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(101), $builtins.hack_string("classname"), $builtins.hack_string("HH\\classname"))
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = n1.?.static_fcall_self()
// CHECK:   ret null
// CHECK: }
function fcall_cls_method(classname<D> $a): void {
  $a::static_fcall_self();
}

// TEST-CHECK-BAL: define $root.fcall_readonly
// CHECK: define $root.fcall_readonly($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $root.g(null)
// CHECK:   n1 = $root.readonly_param(null, $builtins.__sil_readonly(n0))
// CHECK:   ret null
// CHECK: }
function fcall_readonly(): void {
  readonly_param(readonly g());
}

// TEST-CHECK-BAL: type MethCaller$C$b$curry
// CHECK: type MethCaller$C$b$curry = .kind="class" .final {
// CHECK:   arg0: .private *void
// CHECK: }

// TEST-CHECK-BAL: define .final .curry MethCaller$C$b$curry.__invoke
// CHECK: define .final .curry MethCaller$C$b$curry.__invoke(this: *MethCaller$C$b$curry, args: .variadic *HackVec) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *MethCaller$C$b$curry = load &this
// CHECK:   n1: *void = load n0.MethCaller$C$b$curry.arg0
// CHECK:   n2: *HackVec = load &args
// CHECK:   n3 = $builtins.__sil_splat(n2)
// CHECK:   n4 = $root.MethCaller$C$b(null, n1, n3)
// CHECK:   ret n4
// CHECK: }

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
