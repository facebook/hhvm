// RUN: %hackc compile-infer %s --experimental-self-parent-in-trait | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = .kind="class" .static {
// CHECK:   prop3: .public *HackFloat;
// CHECK:   prop4: .public .SomeAttribute *HackMixed;
// CHECK:   MY_CONSTANT: .public .__Infer_Constant__ *HackInt
// CHECK: }

// TEST-CHECK-BAL: "type C "
// CHECK: type C = .kind="class" {
// CHECK:   prop1: .public *HackInt;
// CHECK:   prop2: .public *HackString;
// CHECK:   prop5: .public *HackInt;
// CHECK:   type_: .public *HackInt
// CHECK: }

<<__ConsistentConstruct>>
class C {
  public int $prop1 = 42;
  public string $prop2 = "hello";
  public static float $prop3 = 3.14;
  <<SomeAttribute>>
  public static mixed $prop4 = null;
  public int $prop5 = D::C;

  const int MY_CONSTANT = 7;

  // Test reserved token.
  public int $type = 2;

  // TEST-CHECK-BAL: define C$static.__factory
  // CHECK: define C$static.__factory($this: *C$static) : *C {
  // CHECK: #b0:
  // CHECK:   n0 = __sil_allocate(<C>)
  // CHECK:   ret n0
  // CHECK: }

  // TEST-CHECK-BAL: define C.__construct
  // CHECK: define C.__construct($this: *C, $a: *HackInt, $b: *HackString, $c: *HackInt) : *HackMixed {
  // CHECK: #b0:
  // CHECK:   ret null
  // CHECK: }

  public function __construct(int $a, string $b, int $c) {
  }

  // TEST-CHECK-BAL: define C.cons_static
  // CHECK: define C.cons_static($this: *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *C = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2 = $builtins.hack_get_static_class(n0)
  // CHECK:   n3 = n2.?.__factory()
  // CHECK:   store &$2 <- n3: *HackMixed
  // CHECK:   n4 = $builtins.hhbc_class_has_reified_generics(n1)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b17
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n4)
  // CHECK:   jmp b7
  // CHECK: #b3:
  // CHECK:   prune $builtins.hack_is_true(n4)
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6 = $builtins.hhbc_get_cls_rg_prop(n5)
  // CHECK:   store &$1 <- n6: *HackMixed
  // CHECK:   n7 = $builtins.hhbc_is_type_null(n6)
  // CHECK:   jmp b4, b5
  // CHECK:   .handlers b17
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   jmp b7
  // CHECK: #b5:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n8: *HackMixed = load &$1
  // CHECK:   n9 = $root.count(null, n8)
  // CHECK:   n10 = $builtins.hhbc_cmp_eq(n9, $builtins.hack_int(0))
  // CHECK:   jmp b6, b10
  // CHECK:   .handlers b17
  // CHECK: #b6:
  // CHECK:   prune $builtins.hack_is_true(n10)
  // CHECK:   jmp b7
  // CHECK: #b7:
  // CHECK:   n11: *HackMixed = load &$0
  // CHECK:   n12 = $builtins.hhbc_class_has_reified_generics(n11)
  // CHECK:   jmp b8, b9
  // CHECK:   .handlers b17
  // CHECK: #b8:
  // CHECK:   prune ! $builtins.hack_is_true(n12)
  // CHECK:   jmp b12
  // CHECK: #b9:
  // CHECK:   prune $builtins.hack_is_true(n12)
  // CHECK:   n13: *HackMixed = load &$0
  // CHECK:   n14 = $builtins.hhbc_class_get_c(n13)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b10:
  // CHECK:   prune ! $builtins.hack_is_true(n10)
  // CHECK:   n15: *HackMixed = load &$0
  // CHECK:   n16 = $builtins.hhbc_class_has_reified_generics(n15)
  // CHECK:   jmp b11, b15
  // CHECK:   .handlers b17
  // CHECK: #b11:
  // CHECK:   prune ! $builtins.hack_is_true(n16)
  // CHECK:   jmp b12
  // CHECK: #b12:
  // CHECK:   n17: *HackMixed = load &$0
  // CHECK:   n18 = $builtins.hhbc_has_reified_parent(n17)
  // CHECK:   jmp b13, b14
  // CHECK:   .handlers b17
  // CHECK: #b13:
  // CHECK:   prune ! $builtins.hack_is_true(n18)
  // CHECK:   jmp b16
  // CHECK: #b14:
  // CHECK:   prune $builtins.hack_is_true(n18)
  // CHECK:   n19: *HackMixed = load &$2
  // CHECK:   n20 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n21 = n19.?._86reifiedinit(n20)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b15:
  // CHECK:   prune $builtins.hack_is_true(n16)
  // CHECK:   n22: *HackMixed = load &$2
  // CHECK:   n23: *HackMixed = load &$1
  // CHECK:   n24 = n22.?._86reifiedinit(n23)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b16:
  // CHECK:   n25: *HackMixed = load &$2
  // CHECK:   jmp b18
  // CHECK:   .handlers b17
  // CHECK: #b17(n26: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n27 = $builtins.hhbc_throw(n26)
  // CHECK:   unreachable
  // CHECK: #b18:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n28 = n25.?.__construct($builtins.hack_int(1), $builtins.hack_string("x"), $builtins.hack_int(3))
  // CHECK:   n29 = $builtins.hhbc_lock_obj(n25)
  // CHECK:   store &$a <- n25: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_static(): void {
    $a = new static(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C.cons_self
  // CHECK: define C.cons_self($this: *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *C = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2 = __sil_allocate(<C>)
  // CHECK:   n3 = C._86pinit(n2)
  // CHECK:   store &$2 <- n2: *HackMixed
  // CHECK:   n4 = $builtins.hhbc_class_has_reified_generics(n1)
  // CHECK:   jmp b2, b5
  // CHECK:   .handlers b7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n4)
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6 = $builtins.hhbc_has_reified_parent(n5)
  // CHECK:   jmp b3, b4
  // CHECK:   .handlers b7
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n6)
  // CHECK:   jmp b6
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n6)
  // CHECK:   n7: *HackMixed = load &$2
  // CHECK:   n8 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n9 = n7.?._86reifiedinit(n8)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n4)
  // CHECK:   n10: *HackMixed = load &$0
  // CHECK:   n11 = $builtins.hhbc_class_get_c(n10)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b6:
  // CHECK:   n12: *HackMixed = load &$2
  // CHECK:   jmp b8
  // CHECK:   .handlers b7
  // CHECK: #b7(n13: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n14 = $builtins.hhbc_throw(n13)
  // CHECK:   unreachable
  // CHECK: #b8:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n15 = n12.?.__construct($builtins.hack_int(1), $builtins.hack_string("x"), $builtins.hack_int(3))
  // CHECK:   n16 = $builtins.hhbc_lock_obj(n12)
  // CHECK:   store &$a <- n12: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_self(): void {
    $a = new self(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C.cons_inst
  // CHECK: define C.cons_inst($this: *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *C = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2 = __sil_allocate(<C>)
  // CHECK:   n3 = C._86pinit(n2)
  // CHECK:   store &$2 <- n2: *HackMixed
  // CHECK:   n4 = $builtins.hhbc_class_has_reified_generics(n1)
  // CHECK:   jmp b2, b5
  // CHECK:   .handlers b7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n4)
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6 = $builtins.hhbc_has_reified_parent(n5)
  // CHECK:   jmp b3, b4
  // CHECK:   .handlers b7
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n6)
  // CHECK:   jmp b6
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n6)
  // CHECK:   n7: *HackMixed = load &$2
  // CHECK:   n8 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n9 = n7.?._86reifiedinit(n8)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n4)
  // CHECK:   n10: *HackMixed = load &$0
  // CHECK:   n11 = $builtins.hhbc_class_get_c(n10)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b6:
  // CHECK:   n12: *HackMixed = load &$2
  // CHECK:   jmp b8
  // CHECK:   .handlers b7
  // CHECK: #b7(n13: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n14 = $builtins.hhbc_throw(n13)
  // CHECK:   unreachable
  // CHECK: #b8:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n15 = n12.?.__construct($builtins.hack_int(1), $builtins.hack_string("x"), $builtins.hack_int(3))
  // CHECK:   n16 = $builtins.hhbc_lock_obj(n12)
  // CHECK:   store &$a <- n12: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_inst(): void {
    $a = new C(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C$static.static_signature
  // CHECK: define C$static.static_signature($this: *C$static, $a: *HackMixed, $b: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &$b
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK:   n2 = $builtins.hhbc_cmp_eq(n1, n0)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune ! $builtins.hack_is_true(n2)
  // CHECK:   n3 = $builtins.hhbc_print($builtins.hack_string("unequal"))
  // CHECK:   jmp b3
  // CHECK: #b2:
  // CHECK:   prune $builtins.hack_is_true(n2)
  // CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("equal"))
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   ret null
  // CHECK: }

  // TEST-CHECK-BAL: define C.static_signature
  // CHECK: define C.static_signature($this: *C, $a: *HackMixed, $b: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // forward to the static method
  // CHECK:   n0: *C = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   n2: *HackMixed = load &$a
  // CHECK:   n3: *HackMixed = load &$b
  // CHECK:   n4 = C$static.static_signature(n1, n2, n3)
  // CHECK:   ret n4
  // CHECK: }
  public static function static_signature(mixed $a, mixed $b): void {
    if ($a == $b) {
      echo "equal";
    } else {
      echo "unequal";
    }
  }

  // TEST-CHECK-BAL: define .final C.test_const
  // CHECK: define .final C.test_const($this: *C) : *void {
  // CHECK: local $x: *void
  // CHECK: #b0:
  // CHECK:   n0: *C = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   n2 = $builtins.hack_field_get(n1, "MY_CONSTANT")
  // CHECK:   store &$x <- n2: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public final function test_const(): void {
    $x = C::MY_CONSTANT;
  }

  public static function test_static(): void {}

  // TEST-CHECK-BAL: define C._86pinit
  // CHECK: define C._86pinit($this: *C) : *HackMixed {
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hack_int(42)
  // CHECK:   n1: *HackMixed = load &$this
  // CHECK:   store n1.?.prop1 <- n0: *HackMixed
  // CHECK:   n2 = $builtins.hack_string("hello")
  // CHECK:   store n1.?.prop2 <- n2: *HackMixed
  // CHECK:   n3 = null
  // CHECK:   store n1.?.prop5 <- n3: *HackMixed
  // CHECK:   n4 = $builtins.hack_int(2)
  // CHECK:   store n1.?.type <- n4: *HackMixed
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune $builtins.hack_is_true($builtins.hack_bool(false))
  // CHECK:   jmp b3
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true($builtins.hack_bool(false))
  // CHECK:   n5 = __sil_lazy_class_initialize(<D>)
  // CHECK:   n6 = $builtins.hack_field_get(n5, "C")
  // CHECK:   n7: *HackMixed = load &$this
  // CHECK:   store n7.?.prop5 <- n6: *HackMixed
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK:   ret null
  // CHECK: }
}

// TEST-CHECK-BAL: define C$static._86sinit
// CHECK: define C$static._86sinit($this: *C$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_class_get_c($builtins.hack_string("C"))
// CHECK:   n1 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("prop3"), $builtins.hack_float(3.14))
// CHECK:   n2 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("prop4"), null)
// CHECK:   n3 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("MY_CONSTANT"), $builtins.hack_int(7))
// CHECK:   ret null
// CHECK: }

abstract class AbstractClass {
  // TEST-CHECK-BAL: declare AbstractClass$static.abs_static_func
  // CHECK: declare AbstractClass$static.abs_static_func(*AbstractClass$static, *HackInt, *HackFloat): *HackString
  public static abstract function abs_static_func(int $a, float $b): string;

  // TEST-CHECK-BAL: declare AbstractClass.abs_func
  // CHECK: declare AbstractClass.abs_func(*AbstractClass, *HackInt, *HackFloat): *HackString
  public abstract function abs_func(int $a, float $b): string;
}

trait T0 {
  // TEST-CHECK-BAL: define T0.trait_parent_caller
  // CHECK: define T0.trait_parent_caller($this: *T0, self: *HackMixed, parent: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &parent
  // CHECK:   n1: *T0 = load &$this
  // CHECK:   n2 = __parent__.test_const(n1, n0)
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_caller(): void {
    /* HH_FIXME[4074] This isn't valid Hack but actually occurs in www */
    parent::test_const();
  }
}

trait T1 {
  require extends C;

  // TEST-CHECK-BAL: define T1.trait_parent_caller
  // CHECK: define T1.trait_parent_caller($this: *T1, self: *HackMixed, parent: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &parent
  // CHECK:   n1: *T1 = load &$this
  // CHECK:   n2 = __parent__.test_const(n1, n0)
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_caller(): void {
    parent::test_const();
  }

  // TEST-CHECK-BAL: define T1.trait_parent_static_caller
  // CHECK: define T1.trait_parent_static_caller($this: *T1, self: *HackMixed, parent: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &parent
  // CHECK:   n1: *T1 = load &$this
  // CHECK:   n2 = __parent__.test_static(n1, n0)
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_static_caller(): void {
    parent::test_static();
  }
}

trait T2 {
  // TEST-CHECK-BAL: define T2$static.trait_self_caller
  // CHECK: define T2$static.trait_self_caller($this: *T2$static, self: *HackMixed, parent: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &self
  // CHECK:   n1: *T2$static = load &$this
  // CHECK:   n2 = __self__$static.f(n1, n0)
  // CHECK:   ret null
  // CHECK: }
  public static function trait_self_caller(): void {
    self::f();
  }

  public static function f(): void {}
}

trait T3 {
  // TEST-CHECK-BAL: define T3.trait_self_caller
  // CHECK: define T3.trait_self_caller($this: *T3, self: *HackMixed, parent: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &self
  // CHECK:   n1: *T3 = load &$this
  // CHECK:   n2 = __self__.f(n1, n0)
  // CHECK:   n3 = __self__.g(n1, n0)
  // CHECK:   ret null
  // CHECK: }
  public function trait_self_caller(): void {
    self::f();
    /* HH_FIXME[4090] This isn't valid Hack but actually occurs in www */
    self::g();
  }

  public static function f(): void {}

  public function g(): void {}
}

// TEST-CHECK-BAL: define $root.dynamic_const
// CHECK: define $root.dynamic_const($this: *void, $c: *C) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$c
// CHECK:   n1 = $builtins.hhbc_class_get_c(n0)
// CHECK:   n2 = $builtins.hhbc_cls_cns(n1, $builtins.hack_string("MY_CONSTANT"))
// CHECK:   n3 = $builtins.hhbc_print(n2)
// CHECK:   ret null
// CHECK: }
function dynamic_const(C $c): void {
  echo $c::MY_CONSTANT;
}

// TEST-CHECK-BAL: define $root.cgets
// CHECK: define $root.cgets($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_class_get_c($builtins.hack_string("C"))
// CHECK:   n1 = $builtins.hack_field_get(n0, "prop3")
// CHECK:   n2 = $root.sink(null, n1)
// CHECK:   ret null
// CHECK: }
function cgets(): void {
  sink(C::$prop3);
}

// TEST-CHECK-BAL: define $root.sets
// CHECK: define $root.sets($this: *void) : *void {
// CHECK: local $0: *void, $1: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(3))
// CHECK:   n1 = $builtins.hhbc_class_get_c($builtins.hack_string("C"))
// CHECK:   n2 = $root.source(null)
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK:   store &$1 <- n0: *HackMixed
// CHECK:   n3 = $builtins.hhbc_is_type_struct_c(n2, n0, $builtins.hack_int(1))
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   n5 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("prop3"), n4)
// CHECK:   ret null
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7: *HackMixed = load &$1
// CHECK:   n8 = $builtins.hhbc_throw_as_type_struct_exception(n6, n7)
// CHECK:   unreachable
// CHECK: }
function sets(): void {
  C::$prop3 = source() as float;
}
