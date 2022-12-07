// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = {
// CHECK:   prop3: .public *HackFloat;
// CHECK:   prop4: .public *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "type C "
// CHECK: type C = {
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
  public static mixed $prop4 = null;

  const int MY_CONSTANT = 7;

  // Test reserved token.
  public int $type = 2;

  public function __construct(int $a, int $b, int $c) {
  }

  // TEST-CHECK-BAL: define C.cons_static
  // CHECK: define C.cons_static($this: *C) : *HackMixed {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void, base: *HackMixed
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *C = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK:   n2: *C = load &$this
  // CHECK:   n3 = $builtins.hack_get_static_class(n2)
  // CHECK:   n4 = n3.HackMixed.__factory()
  // CHECK:   store &$2 <- n4: *HackMixed
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6 = $builtins.hhbc_class_has_reified_generics(n5)
  // CHECK:   jmp b2, b3
  // CHECK:   .handlers b17
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n6)
  // CHECK:   jmp b7
  // CHECK: #b3:
  // CHECK:   prune $builtins.hack_is_true(n6)
  // CHECK:   n7: *HackMixed = load &$0
  // CHECK:   n8 = $builtins.hhbc_get_cls_rg_prop(n7)
  // CHECK:   store &$1 <- n8: *HackMixed
  // CHECK:   n9 = $builtins.hhbc_is_type_null(n8)
  // CHECK:   jmp b4, b5
  // CHECK:   .handlers b17
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n9)
  // CHECK:   jmp b7
  // CHECK: #b5:
  // CHECK:   prune ! $builtins.hack_is_true(n9)
  // CHECK:   n10: *HackMixed = load &$1
  // CHECK:   n11 = $root.count(null, n10)
  // CHECK:   n12 = $builtins.hhbc_cmp_eq(n11, $builtins.hack_int(0))
  // CHECK:   jmp b6, b10
  // CHECK:   .handlers b17
  // CHECK: #b6:
  // CHECK:   prune $builtins.hack_is_true(n12)
  // CHECK:   jmp b7
  // CHECK: #b7:
  // CHECK:   n13: *HackMixed = load &$0
  // CHECK:   n14 = $builtins.hhbc_class_has_reified_generics(n13)
  // CHECK:   jmp b8, b9
  // CHECK:   .handlers b17
  // CHECK: #b8:
  // CHECK:   prune ! $builtins.hack_is_true(n14)
  // CHECK:   jmp b12
  // CHECK: #b9:
  // CHECK:   prune $builtins.hack_is_true(n14)
  // CHECK:   n15: *HackMixed = load &$0
  // CHECK:   n16 = $builtins.hhbc_class_get_c(n15)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b10:
  // CHECK:   prune ! $builtins.hack_is_true(n12)
  // CHECK:   n17: *HackMixed = load &$0
  // CHECK:   n18 = $builtins.hhbc_class_has_reified_generics(n17)
  // CHECK:   jmp b11, b15
  // CHECK:   .handlers b17
  // CHECK: #b11:
  // CHECK:   prune ! $builtins.hack_is_true(n18)
  // CHECK:   jmp b12
  // CHECK: #b12:
  // CHECK:   n19: *HackMixed = load &$0
  // CHECK:   n20 = $builtins.hhbc_has_reified_parent(n19)
  // CHECK:   jmp b13, b14
  // CHECK:   .handlers b17
  // CHECK: #b13:
  // CHECK:   prune ! $builtins.hack_is_true(n20)
  // CHECK:   jmp b16
  // CHECK: #b14:
  // CHECK:   prune $builtins.hack_is_true(n20)
  // CHECK:   n21: *HackMixed = load &$2
  // CHECK:   n22 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n23 = n21.HackMixed._86reifiedinit(n22)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b15:
  // CHECK:   prune $builtins.hack_is_true(n18)
  // CHECK:   n24: *HackMixed = load &$2
  // CHECK:   n25: *HackMixed = load &$1
  // CHECK:   n26 = n24.HackMixed._86reifiedinit(n25)
  // CHECK:   jmp b16
  // CHECK:   .handlers b17
  // CHECK: #b16:
  // CHECK:   n27: *HackMixed = load &$2
  // CHECK:   jmp b18
  // CHECK:   .handlers b17
  // CHECK: #b17(n28: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n29 = $builtins.hhbc_throw(n28)
  // CHECK:   unreachable
  // CHECK: #b18:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n30 = n27.HackMixed.__construct($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
  // CHECK:   n31 = $builtins.hhbc_lock_obj(n27)
  // CHECK:   store &$a <- n27: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_static(): void {
    $a = new static(1, 2, 3);
  }

  // TEST-CHECK-BAL: define C.cons_self
  // CHECK: define C.cons_self($this: *C) : *HackMixed {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void, base: *HackMixed
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *C$static = load &C$static::static_singleton
  // CHECK:   n1 = $builtins.lazy_initialize(n0)
  // CHECK:   store &$0 <- n0: *HackMixed
  // CHECK:   n2: *C$static = load &C$static::static_singleton
  // CHECK:   n3 = $builtins.lazy_initialize(n2)
  // CHECK:   n4 = n2.HackMixed.__factory()
  // CHECK:   store &$2 <- n4: *HackMixed
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6 = $builtins.hhbc_class_has_reified_generics(n5)
  // CHECK:   jmp b2, b5
  // CHECK:   .handlers b7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n6)
  // CHECK:   n7: *HackMixed = load &$0
  // CHECK:   n8 = $builtins.hhbc_has_reified_parent(n7)
  // CHECK:   jmp b3, b4
  // CHECK:   .handlers b7
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n8)
  // CHECK:   jmp b6
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n8)
  // CHECK:   n9: *HackMixed = load &$2
  // CHECK:   n10 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n11 = n9.HackMixed._86reifiedinit(n10)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n6)
  // CHECK:   n12: *HackMixed = load &$0
  // CHECK:   n13 = $builtins.hhbc_class_get_c(n12)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b6:
  // CHECK:   n14: *HackMixed = load &$2
  // CHECK:   jmp b8
  // CHECK:   .handlers b7
  // CHECK: #b7(n15: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n16 = $builtins.hhbc_throw(n15)
  // CHECK:   unreachable
  // CHECK: #b8:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n17 = n14.HackMixed.__construct($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
  // CHECK:   n18 = $builtins.hhbc_lock_obj(n14)
  // CHECK:   store &$a <- n14: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_self(): void {
    $a = new self(1, 2, 3);
  }

  // TEST-CHECK-BAL: define C.cons_inst
  // CHECK: define C.cons_inst($this: *C) : *HackMixed {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void, base: *HackMixed
  // CHECK: #b0:
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *C$static = load &C$static::static_singleton
  // CHECK:   n1 = $builtins.lazy_initialize(n0)
  // CHECK:   store &$0 <- n0: *HackMixed
  // CHECK:   n2: *C$static = load &C$static::static_singleton
  // CHECK:   n3 = $builtins.lazy_initialize(n2)
  // CHECK:   n4 = n2.HackMixed.__factory()
  // CHECK:   store &$2 <- n4: *HackMixed
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK:   n6 = $builtins.hhbc_class_has_reified_generics(n5)
  // CHECK:   jmp b2, b5
  // CHECK:   .handlers b7
  // CHECK: #b2:
  // CHECK:   prune ! $builtins.hack_is_true(n6)
  // CHECK:   n7: *HackMixed = load &$0
  // CHECK:   n8 = $builtins.hhbc_has_reified_parent(n7)
  // CHECK:   jmp b3, b4
  // CHECK:   .handlers b7
  // CHECK: #b3:
  // CHECK:   prune ! $builtins.hack_is_true(n8)
  // CHECK:   jmp b6
  // CHECK: #b4:
  // CHECK:   prune $builtins.hack_is_true(n8)
  // CHECK:   n9: *HackMixed = load &$2
  // CHECK:   n10 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
  // CHECK:   n11 = n9.HackMixed._86reifiedinit(n10)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b5:
  // CHECK:   prune $builtins.hack_is_true(n6)
  // CHECK:   n12: *HackMixed = load &$0
  // CHECK:   n13 = $builtins.hhbc_class_get_c(n12)
  // CHECK:   jmp b6
  // CHECK:   .handlers b7
  // CHECK: #b6:
  // CHECK:   n14: *HackMixed = load &$2
  // CHECK:   jmp b8
  // CHECK:   .handlers b7
  // CHECK: #b7(n15: *HackMixed):
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n16 = $builtins.hhbc_throw(n15)
  // CHECK:   unreachable
  // CHECK: #b8:
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK:   n17 = n14.HackMixed.__construct($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
  // CHECK:   n18 = $builtins.hhbc_lock_obj(n14)
  // CHECK:   store &$a <- n14: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function cons_inst(): void {
    $a = new C(1, 2, 3);
  }

  // TEST-CHECK-BAL: define C$static.static_signature
  // CHECK: define C$static.static_signature($this: *C$static, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
  // CHECK: local base: *HackMixed
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
  // CHECK: define C.static_signature($this: *C, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
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
  // CHECK: define C.test_const($this: *C) : *HackMixed {
  // CHECK: local $x: *void, base: *HackMixed
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &C$static::MY_CONSTANT
  // CHECK:   store &$x <- n0: *HackMixed
  // CHECK:   ret null
  // CHECK: }
  public function test_const(): void {
    $x = C::MY_CONSTANT;
  }
}

// TEST-CHECK-BAL: global C$static::MY_CONSTANT
// CHECK: global C$static::MY_CONSTANT : *HackMixed

// TEST-CHECK-BAL: global C$static::static_singleton
// CHECK: global C$static::static_singleton : *C$static
