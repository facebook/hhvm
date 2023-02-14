// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = .kind="class" .static {
// CHECK:   prop3: .public *HackFloat;
// CHECK:   prop4: .public .SomeAttribute *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "type C "
// CHECK: type C = .kind="class" {
// CHECK:   prop1: .public *HackInt;
// CHECK:   prop2: .public *HackString;
// CHECK:   type_: .public *HackInt
// CHECK: }

// TEST-CHECK-BAL: define C$static.$init_static()
// CHECK: define C$static.$init_static() : void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.alloc_words(0)
// CHECK:   store &C$static::static_singleton <- n0: *C$static
// CHECK:   store &C$static::MY_CONSTANT <- $builtins.hack_int(7): *HackMixed
// CHECK:   ret 0
// CHECK: }

<<__ConsistentConstruct>>
class C {
  public int $prop1 = 42;
  public string $prop2 = "hello";
  public static float $prop3 = 3.14;
  <<SomeAttribute>>
  public static mixed $prop4 = null;

  const int MY_CONSTANT = 7;

  // Test reserved token.
  public int $type = 2;

  // TEST-CHECK-BAL: define C$static.__factory(
  // CHECK: define C$static.__factory($this: *C$static, $a: *HackInt, $b: *HackString, $c: *HackInt) : *C {
  // CHECK: #b0:
  // CHECK:   n0: *HackInt = load &$a
  // CHECK:   n1: *HackString = load &$b
  // CHECK:   n2: *HackInt = load &$c
  // CHECK:   n3 = __sil_allocate(<C>)
  // CHECK:   n4 = C.__construct(n3, n0, n1, n2)
  // CHECK:   ret n3
  // CHECK: }

  // TEST-CHECK-BAL: define C.__construct(
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
  // CHECK:   n0 = $builtins.hack_string("x")
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n1: *C = load &$this
  // CHECK:   n2 = $builtins.hack_get_static_class(n1)
  // CHECK:   store &$0 <- n2: *HackMixed
  // CHECK:   n3: *C = load &$this
  // CHECK:   n4 = $builtins.hack_get_static_class(n3)
  // CHECK:   n5 = n4.HackMixed.__factory()
  // CHECK:   store &$2 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK:   n7 = $builtins.hhbc_class_has_reified_generics(n6)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b17
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   jmp b7
  // CHECK: #b3:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   n8: *HackMixed = load &$0
  // CHECK:   n9 = $builtins.hhbc_get_cls_rg_prop(n8)
  // CHECK:   store &$1 <- n9: *HackMixed
  // CHECK:   n10 = $builtins.hhbc_is_type_null(n9)
  // CHECK:   jmp b4, b5
  // CHECK:   .handlers b17
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n10)
  // CHECK:   jmp b7
  // CHECK: #b5:
  // CHECK:   prune ! $builtins.hack_is_true(n10)
  // CHECK:   n11: *HackMixed = load &$1
  // CHECK:   n12 = $root.count(null, n11)
  // CHECK:   n13 = $builtins.hhbc_cmp_eq(n12, $builtins.hack_int(0))
  // CHECK:   jmp b6, b10
  // CHECK:   .handlers b17
  // CHECK: #b6:
  // CHECK:   prune $builtins.hack_is_true(n13)
  // CHECK:   jmp b7
  // CHECK: #b7:
  // CHECK:   n14: *HackMixed = load &$0
  // CHECK:   n15 = $builtins.hhbc_class_has_reified_generics(n14)
  // CHECK:   jmp b8, b9
  // CHECK:   .handlers b17
  // CHECK: #b8:
  // CHECK:   prune ! $builtins.hack_is_true(n15)
  // CHECK:   jmp b12
  // CHECK: #b9:
  // CHECK:   prune $builtins.hack_is_true(n15)
  // CHECK:   n16: *HackMixed = load &$0
  // CHECK:   n17 = $builtins.hhbc_class_get_c(n16)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b10:
  // CHECK:   prune ! $builtins.hack_is_true(n13)
  // CHECK:   n18: *HackMixed = load &$0
  // CHECK:   n19 = $builtins.hhbc_class_has_reified_generics(n18)
  // CHECK:   jmp b11, b15
  // CHECK:   .handlers b17
  // CHECK: #b11:
  // CHECK:   prune ! $builtins.hack_is_true(n19)
  // CHECK:   jmp b12
  // CHECK: #b12:
  // CHECK:   n20: *HackMixed = load &$0
  // CHECK:   n21 = $builtins.hhbc_has_reified_parent(n20)
  // CHECK:   jmp b13, b14
  // CHECK:   .handlers b17
  // CHECK: #b13:
  // CHECK:   prune ! $builtins.hack_is_true(n21)
  // CHECK:   jmp b16
  // CHECK: #b14:
  // CHECK:   prune $builtins.hack_is_true(n21)
  // CHECK:   n22: *HackMixed = load &$2
  // CHECK:   n23 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n24 = n22.HackMixed._86reifiedinit(n23)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b15:
  // CHECK:   prune $builtins.hack_is_true(n19)
  // CHECK:   n25: *HackMixed = load &$2
  // CHECK:   n26: *HackMixed = load &$1
  // CHECK:   n27 = n25.HackMixed._86reifiedinit(n26)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b16:
  // CHECK:   n28: *HackMixed = load &$2
  // CHECK:   jmp b18
  // CHECK:   .handlers b17
  // CHECK: #b17(n29: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n30 = $builtins.hhbc_throw(n29)
  // CHECK:   unreachable
  // CHECK: #b18:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n31 = n28.HackMixed.__construct($builtins.hack_int(1), n0, $builtins.hack_int(3))
  // CHECK:   n32 = $builtins.hhbc_lock_obj(n28)
  // CHECK:   store &$a <- n28: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_static(): void {
    $a = new static(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C.cons_self
  // CHECK: define C.cons_self($this: *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hack_string("x")
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n1: *C$static = load &C$static::static_singleton
  // CHECK:   n2 = $builtins.lazy_initialize(n1)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n3: *C$static = load &C$static::static_singleton
  // CHECK:   n4 = $builtins.lazy_initialize(n3)
  // CHECK:   n5 = n3.HackMixed.__factory()
  // CHECK:   store &$2 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK:   n7 = $builtins.hhbc_class_has_reified_generics(n6)
  // CHECK:   jmp b2, b5
  // CHECK:   .handlers b7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n8: *HackMixed = load &$0
  // CHECK:   n9 = $builtins.hhbc_has_reified_parent(n8)
  // CHECK:   jmp b3, b4
  // CHECK:   .handlers b7
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n9)
  // CHECK:   jmp b6
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n9)
  // CHECK:   n10: *HackMixed = load &$2
  // CHECK:   n11 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n12 = n10.HackMixed._86reifiedinit(n11)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   n13: *HackMixed = load &$0
  // CHECK:   n14 = $builtins.hhbc_class_get_c(n13)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b6:
  // CHECK:   n15: *HackMixed = load &$2
  // CHECK:   jmp b8
  // CHECK:   .handlers b7
  // CHECK: #b7(n16: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n17 = $builtins.hhbc_throw(n16)
  // CHECK:   unreachable
  // CHECK: #b8:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n18 = n15.HackMixed.__construct($builtins.hack_int(1), n0, $builtins.hack_int(3))
  // CHECK:   n19 = $builtins.hhbc_lock_obj(n15)
  // CHECK:   store &$a <- n15: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_self(): void {
    $a = new self(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C.cons_inst
  // CHECK: define C.cons_inst($this: *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hack_string("x")
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n1: *C$static = load &C$static::static_singleton
  // CHECK:   n2 = $builtins.lazy_initialize(n1)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n3: *C$static = load &C$static::static_singleton
  // CHECK:   n4 = $builtins.lazy_initialize(n3)
  // CHECK:   n5 = n3.HackMixed.__factory()
  // CHECK:   store &$2 <- n5: *HackMixed
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK:   n7 = $builtins.hhbc_class_has_reified_generics(n6)
  // CHECK:   jmp b2, b5
  // CHECK:   .handlers b7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n7)
  // CHECK:   n8: *HackMixed = load &$0
  // CHECK:   n9 = $builtins.hhbc_has_reified_parent(n8)
  // CHECK:   jmp b3, b4
  // CHECK:   .handlers b7
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n9)
  // CHECK:   jmp b6
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n9)
  // CHECK:   n10: *HackMixed = load &$2
  // CHECK:   n11 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n12 = n10.HackMixed._86reifiedinit(n11)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n7)
  // CHECK:   n13: *HackMixed = load &$0
  // CHECK:   n14 = $builtins.hhbc_class_get_c(n13)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b6:
  // CHECK:   n15: *HackMixed = load &$2
  // CHECK:   jmp b8
  // CHECK:   .handlers b7
  // CHECK: #b7(n16: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n17 = $builtins.hhbc_throw(n16)
  // CHECK:   unreachable
  // CHECK: #b8:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n18 = n15.HackMixed.__construct($builtins.hack_int(1), n0, $builtins.hack_int(3))
  // CHECK:   n19 = $builtins.hhbc_lock_obj(n15)
  // CHECK:   store &$a <- n15: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_inst(): void {
    $a = new C(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C$static.static_signature
  // CHECK: define C$static.static_signature($this: *C$static, $a: *HackMixed, $b: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hack_string("equal")
  // CHECK:   n1 = $builtins.hack_string("unequal")
  // CHECK:   n2: *HackMixed = load &$b
  // CHECK:   n3: *HackMixed = load &$a
  // CHECK:   n4 = $builtins.hhbc_cmp_eq(n3, n2)
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK:   prune ! $builtins.hack_is_true(n4)
  // CHECK:   n5 = $builtins.hhbc_print(n1)
  // CHECK:   jmp b3
  // CHECK: #b2:
  // CHECK:   prune $builtins.hack_is_true(n4)
  // CHECK:   n6 = $builtins.hhbc_print(n0)
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

  // TEST-CHECK-BAL: define C.test_const
  // CHECK: define C.test_const($this: *C) : *void {
  // CHECK: local $x: *void
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &C$static::MY_CONSTANT
  // CHECK:   store &$x <- n0: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function test_const(): void {
    $x = C::MY_CONSTANT;
  }
}

trait T0 {
  // TEST-CHECK-BAL: define T0.trait_parent_caller
  // CHECK: define T0.trait_parent_caller($this: *T0) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *T0 = load &$this
  // CHECK:   n1 = __parent__.test_const(n0)
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
  // CHECK: define T1.trait_parent_caller($this: *T1) : *void {
  // CHECK: #b0:
  // CHECK:   n0: *T1 = load &$this
  // CHECK:   n1 = __parent__.test_const(n0)
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_caller(): void {
    parent::test_const();
  }
}

// TEST-CHECK-BAL: define $root.dynamic_const
// CHECK: define $root.dynamic_const($this: *void, $c: *C) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("MY_CONSTANT")
// CHECK:   n1: *HackMixed = load &$c
// CHECK:   n2 = $builtins.hhbc_class_get_c(n1)
// CHECK:   n3 = $builtins.hhbc_cls_cns(n2, n0)
// CHECK:   n4 = $builtins.hhbc_print(n3)
// CHECK:   ret null
// CHECK: }
function dynamic_const(C $c): void {
  echo $c::MY_CONSTANT;
}

// TEST-CHECK-BAL: global C$static::MY_CONSTANT
// CHECK: global C$static::MY_CONSTANT : *HackMixed

// TEST-CHECK-BAL: global C$static::static_singleton
// CHECK: global C$static::static_singleton : *C$static
