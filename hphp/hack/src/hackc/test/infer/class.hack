// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static extends HH::classname = .kind="class" .static {
// CHECK:   prop3: .public *HackFloat;
// CHECK:   prop4: .public .SomeAttribute *HackMixed;
// CHECK:   MY_CONSTANT: .public .constant *HackInt
// CHECK: }

// TEST-CHECK-BAL: "type C "
// CHECK: type C = .kind="class" {
// CHECK:   prop1: .public *HackInt;
// CHECK:   prop2: .public *HackString;
// CHECK:   prop5: .public *HackInt;
// CHECK:   mangled:::type: .public *HackInt
// CHECK: }

// TEST-CHECK-BAL: define C$static.__factory
// CHECK: define C$static.__factory($this: *C$static) : *C {
// CHECK: #b0:
// CHECK:   n0 = __sil_allocate(<C>)
// CHECK:   n1 = C._86pinit(n0)
// CHECK:   ret n0
// CHECK: }

// TEST-CHECK-BAL: define C$static._86constinit
// CHECK: define C$static._86constinit($this: .notnull *C$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_class_get_c($builtins.hack_string("C"))
// CHECK:   n1 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("MY_CONSTANT"), $builtins.hack_int(7))
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define C._86pinit
// CHECK: define C._86pinit($this: .notnull *C) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1 = $builtins.hack_int(42)
// CHECK:   store n0.?.prop1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3 = $builtins.hack_string("hello")
// CHECK:   store n2.?.prop2 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5 = null
// CHECK:   store n4.?.prop5 <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$this
// CHECK:   n7 = $builtins.hack_int(2)
// CHECK:   store n6.?.mangled:::type <- n7: *HackMixed
// CHECK: // .column 23
// CHECK:   n8 = __sil_lazy_class_initialize(<D>)
// CHECK:   n9 = $builtins.hack_field_get(n8, "C")
// CHECK: // .column 1
// CHECK:   n10: *HackMixed = load &$this
// CHECK:   store n10.?.prop5 <- n9: *HackMixed
// CHECK: // .column 1
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define C$static._86sinit
// CHECK: define C$static._86sinit($this: .notnull *C$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *C$static = load &$this
// CHECK:   n1 = C$static._86constinit(n0)
// CHECK:   n2 = $builtins.hhbc_class_get_c($builtins.hack_string("C"))
// CHECK:   n3 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("prop3"), $builtins.hack_float(3.14))
// CHECK:   n4 = $builtins.hack_set_static_prop($builtins.hack_string("C"), $builtins.hack_string("prop4"), null)
// CHECK:   ret null
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

  // TEST-CHECK-BAL: define C.__construct
  // CHECK: define C.__construct($this: .notnull *C, $a: .notnull *HackInt, $b: .notnull *HackString, $c: .notnull *HackInt) : *HackMixed {
  // CHECK: #b0:
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }

  public function __construct(int $a, string $b, int $c) {
  }

  // TEST-CHECK-BAL: define C.cons_static
  // CHECK: define C.cons_static($this: .notnull *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hhbc_new_vec()
  // CHECK: // .column 10
  // CHECK:   n1: *C = load &$this
  // CHECK: // .column 10
  // CHECK:   n2 = $builtins.hack_get_static_class(n1)
  // CHECK: // .column 10
  // CHECK:   store &$0 <- n2: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n3: *C = load &$this
  // CHECK: // .column 10
  // CHECK:   n4 = $builtins.hack_get_static_class(n3)
  // CHECK: // .column 10
  // CHECK:   n5 = n4.?.__factory()
  // CHECK: // .column 10
  // CHECK:   store &$2 <- n5: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n6: *HackMixed = load &$0
  // CHECK: // .column 10
  // CHECK:   store &$1 <- n0: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n7: *HackMixed = load &$2
  // CHECK: // .column 10
  // CHECK:   n8: *HackMixed = load &$0
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n9: *HackMixed = load &$2
  // CHECK: // .column 10
  // CHECK:   jmp b2
  // CHECK:   .handlers b1
  // CHECK: #b1(n10: *HackMixed):
  // CHECK: // .column 10
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   throw n10
  // CHECK: #b2:
  // CHECK: // .column 10
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n11 = n9.?.__construct($builtins.hack_int(1), $builtins.hack_string("x"), $builtins.hack_int(3))
  // CHECK: // .column 10
  // CHECK:   n12 = $builtins.hhbc_lock_obj(n9)
  // CHECK: // .column 5
  // CHECK:   store &$a <- n9: *HackMixed
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function cons_static(): void {
    $a = new static(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C.cons_self
  // CHECK: define C.cons_self($this: .notnull *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hhbc_new_vec()
  // CHECK: // .column 10
  // CHECK:   n1 = __sil_lazy_class_initialize(<C>)
  // CHECK: // .column 10
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n2 = __sil_allocate(<C>)
  // CHECK:   n3 = C._86pinit(n2)
  // CHECK: // .column 10
  // CHECK:   store &$2 <- n2: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- n0: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n4: *HackMixed = load &$2
  // CHECK: // .column 10
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n6: *HackMixed = load &$2
  // CHECK: // .column 10
  // CHECK:   jmp b2
  // CHECK:   .handlers b1
  // CHECK: #b1(n7: *HackMixed):
  // CHECK: // .column 10
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   throw n7
  // CHECK: #b2:
  // CHECK: // .column 10
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n8 = n6.?.__construct($builtins.hack_int(1), $builtins.hack_string("x"), $builtins.hack_int(3))
  // CHECK: // .column 10
  // CHECK:   n9 = $builtins.hhbc_lock_obj(n6)
  // CHECK: // .column 5
  // CHECK:   store &$a <- n6: *HackMixed
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function cons_self(): void {
    $a = new self(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C.cons_inst
  // CHECK: define C.cons_inst($this: .notnull *C) : *void {
  // CHECK: local $a: *void, $0: *void, $1: *void, $2: *void
  // CHECK: #b0:
  // CHECK:   n0 = $builtins.hhbc_new_vec()
  // CHECK: // .column 10
  // CHECK:   n1 = __sil_lazy_class_initialize(<C>)
  // CHECK: // .column 10
  // CHECK:   store &$0 <- n1: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n2 = __sil_allocate(<C>)
  // CHECK:   n3 = C._86pinit(n2)
  // CHECK: // .column 10
  // CHECK:   store &$2 <- n2: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- n0: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n4: *HackMixed = load &$2
  // CHECK: // .column 10
  // CHECK:   n5: *HackMixed = load &$0
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n6: *HackMixed = load &$2
  // CHECK: // .column 10
  // CHECK:   jmp b2
  // CHECK:   .handlers b1
  // CHECK: #b1(n7: *HackMixed):
  // CHECK: // .column 10
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   throw n7
  // CHECK: #b2:
  // CHECK: // .column 10
  // CHECK:   store &$0 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$1 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$2 <- null: *HackMixed
  // CHECK: // .column 10
  // CHECK:   n8 = n6.?.__construct($builtins.hack_int(1), $builtins.hack_string("x"), $builtins.hack_int(3))
  // CHECK: // .column 10
  // CHECK:   n9 = $builtins.hhbc_lock_obj(n6)
  // CHECK: // .column 5
  // CHECK:   store &$a <- n6: *HackMixed
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function cons_inst(): void {
    $a = new C(1, "x", 3);
  }

  // TEST-CHECK-BAL: define C$static.static_signature
  // CHECK: define C$static.static_signature($this: .notnull *C$static, $a: *HackMixed, $b: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 15
  // CHECK:   n0: *HackMixed = load &$b
  // CHECK: // .column 15
  // CHECK:   n1: *HackMixed = load &$a
  // CHECK: // .column 15
  // CHECK:   n2 = $builtins.hhbc_cmp_eq(n1, n0)
  // CHECK: // .column 15
  // CHECK:   jmp b1, b2
  // CHECK: #b1:
  // CHECK: // .column 15
  // CHECK:   prune ! $builtins.hack_is_true(n2)
  // CHECK: // .column 7
  // CHECK:   n3 = $builtins.hhbc_print($builtins.hack_string("unequal"))
  // CHECK: // .column 7
  // CHECK:   jmp b3
  // CHECK: #b2:
  // CHECK: // .column 15
  // CHECK:   prune $builtins.hack_is_true(n2)
  // CHECK: // .column 7
  // CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("equal"))
  // CHECK: // .column 5
  // CHECK:   jmp b3
  // CHECK: #b3:
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }

  // TEST-CHECK-BAL: define .wrapper C.static_signature
  // CHECK: define .wrapper C.static_signature($this: .notnull *C, $a: *HackMixed, $b: *HackMixed) : *void {
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
  // CHECK: define .final C.test_const($this: .notnull *C) : *void {
  // CHECK: local $x: *void
  // CHECK: #b0:
  // CHECK: // .column 10
  // CHECK:   n0 = __sil_lazy_class_initialize(<C>)
  // CHECK:   n1 = $builtins.hack_field_get(n0, "MY_CONSTANT")
  // CHECK: // .column 5
  // CHECK:   store &$x <- n1: *HackMixed
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public final function test_const(): void {
    $x = C::MY_CONSTANT;
  }

  public static function test_static(): void {}

}

// TEST-CHECK-BAL: type AbstractClass
// CHECK: type AbstractClass = .kind="class" .abstract {
// CHECK: }
abstract class AbstractClass {
  // TEST-CHECK-BAL: declare .abstract AbstractClass$static.abs_static_func
  // CHECK: declare .abstract AbstractClass$static.abs_static_func(*AbstractClass$static, *HackInt, *HackFloat): *HackString
  public static abstract function abs_static_func(int $a, float $b): string;

  // TEST-CHECK-BAL: declare .abstract AbstractClass.abs_func
  // CHECK: declare .abstract AbstractClass.abs_func(*AbstractClass, *HackInt, *HackFloat): *HackString
  public abstract function abs_func(int $a, float $b): string;
}

trait T0 {
  // TEST-CHECK-BAL: define T0.trait_parent_caller
  // CHECK: define T0.trait_parent_caller($this: .notnull *T0, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *T0 = load &$this
  // CHECK:   n1 = __parent__.test_const(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_caller(): void {
    /* HH_FIXME[4074] This isn't valid Hack but actually occurs in www */
    parent::test_const();
  }

  // TEST-CHECK-BAL: define .wrapper T0$static.with_optional_argument
  // CHECK: define .wrapper T0$static.with_optional_argument($this: .notnull *T0$static, self: *HackMixed) : *void {
  // CHECK: local $opt1: *void, $opt2: *void
  // CHECK: #b0:
  // CHECK: // .column 53
  // CHECK:   store &$opt1 <- $builtins.hack_int(0): *HackMixed
  // CHECK: // .column 66
  // CHECK:   store &$opt2 <- $builtins.hack_int(1): *HackMixed
  // CHECK: // .column 66
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *HackMixed = load &$opt1
  // CHECK:   n1: *HackMixed = load &$opt2
  // CHECK:   n2: *HackMixed = load &$this
  // CHECK:   n3 = n2.?.with_optional_argument(n0, n1)
  // CHECK:   ret n3
  // CHECK: }
  // TEST-CHECK-BAL: define .wrapper T0.with_optional_argument
  // CHECK: define .wrapper T0.with_optional_argument($this: .notnull *T0, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // forward to the static method
  // CHECK:   n0: *T0 = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   n2 = T0$static.with_optional_argument(n1)
  // CHECK:   ret n2
  // CHECK: }
  // TEST-CHECK-BAL: define .wrapper T0$static.with_optional_argument
  // CHECK: define .wrapper T0$static.with_optional_argument($this: .notnull *T0$static, $opt1: .notnull *HackInt, self: *HackMixed) : *void {
  // CHECK: local $opt2: *void
  // CHECK: #b0:
  // CHECK: // .column 66
  // CHECK:   store &$opt2 <- $builtins.hack_int(1): *HackMixed
  // CHECK: // .column 66
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *HackMixed = load &$opt1
  // CHECK:   n1: *HackMixed = load &$opt2
  // CHECK:   n2: *HackMixed = load &$this
  // CHECK:   n3 = n2.?.with_optional_argument(n0, n1)
  // CHECK:   ret n3
  // CHECK: }
  // TEST-CHECK-BAL: define .wrapper T0.with_optional_argument
  // CHECK: define .wrapper T0.with_optional_argument($this: .notnull *T0, $opt1: .notnull *HackInt, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // forward to the static method
  // CHECK:   n0: *T0 = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   n2: *HackInt = load &$opt1
  // CHECK:   n3 = T0$static.with_optional_argument(n1, n2)
  // CHECK:   ret n3
  // CHECK: }
  // TEST-CHECK-BAL: define T0$static.with_optional_argument
  // CHECK: define T0$static.with_optional_argument($this: .notnull *T0$static, $opt1: .notnull *HackInt, $opt2: .notnull *HackInt, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  // TEST-CHECK-BAL: define .wrapper T0.with_optional_argument
  // CHECK: define .wrapper T0.with_optional_argument($this: .notnull *T0, $opt1: .notnull *HackInt, $opt2: .notnull *HackInt, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // forward to the static method
  // CHECK:   n0: *T0 = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   n2: *HackInt = load &$opt1
  // CHECK:   n3: *HackInt = load &$opt2
  // CHECK:   n4 = T0$static.with_optional_argument(n1, n2, n3)
  // CHECK:   ret n4
  // CHECK: }
  public static function with_optional_argument(int $opt1=0, int $opt2=1): void {
  }
}

trait T1 {
  require extends C;

  // TEST-CHECK-BAL: define T1.trait_parent_caller
  // CHECK: define T1.trait_parent_caller($this: .notnull *T1, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *T1 = load &$this
  // CHECK:   n1 = __parent__.test_const(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_caller(): void {
    parent::test_const();
  }

  // TEST-CHECK-BAL: define T1.trait_parent_static_caller
  // CHECK: define T1.trait_parent_static_caller($this: .notnull *T1, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *T1 = load &$this
  // CHECK:   n1 = __parent__.test_static(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function trait_parent_static_caller(): void {
    parent::test_static();
  }
}

trait T2 {
  // TEST-CHECK-BAL: define T2$static.trait_self_caller
  // CHECK: define T2$static.trait_self_caller($this: .notnull *T2$static, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *T2$static = load &$this
  // CHECK:   n1 = __self__$static.f(n0)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }

  // TEST-CHECK-BAL: define .wrapper T2.trait_self_caller
  // CHECK: define .wrapper T2.trait_self_caller($this: .notnull *T2, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // forward to the static method
  // CHECK:   n0: *T2 = load &$this
  // CHECK:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK:   n2 = T2$static.trait_self_caller(n1)
  // CHECK:   ret n2
  // CHECK: }
  public static function trait_self_caller(): void {
    self::f();
  }

  public static function f(): void {}
}

trait T3 {
  // TEST-CHECK-BAL: define T3.trait_self_caller
  // CHECK: define T3.trait_self_caller($this: .notnull *T3, self: *HackMixed) : *void {
  // CHECK: #b0:
  // CHECK: // .column 5
  // CHECK:   n0: *T3 = load &$this
  // CHECK:   n1 = __self__.f(n0)
  // CHECK: // .column 5
  // CHECK:   n2: *T3 = load &$this
  // CHECK:   n3 = __self__.g(n2)
  // CHECK: // .column 4
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


// TEST-CHECK-BAL: define T4$static._86constinit
// CHECK: define T4$static._86constinit($this: .notnull *T4$static, self: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))
// CHECK:   n1 = $builtins.hhbc_class_get_c($builtins.hack_string("T4"))
// CHECK:   n2 = $builtins.hack_set_static_prop($builtins.hack_string("T4"), $builtins.hack_string("MY_CONSTANT"), $builtins.hack_int(42))
// CHECK:   n3 = $builtins.hack_set_static_prop($builtins.hack_string("T4"), $builtins.hack_string("T"), n0)
// CHECK:   ret null
// CHECK: }
trait T4 {
  const int MY_CONSTANT = 42;
  const type T = int;
}

// TEST-CHECK-BAL: define $root.dynamic_const
// CHECK: define $root.dynamic_const($this: *void, $c: *C) : *void {
// CHECK: #b0:
// CHECK: // .column 8
// CHECK:   n0: *HackMixed = load &$c
// CHECK: // .column 8
// CHECK:   n1 = $builtins.hhbc_class_get_c(n0)
// CHECK: // .column 8
// CHECK:   n2 = $builtins.hhbc_cls_cns(n1, $builtins.hack_string("MY_CONSTANT"))
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_print(n2)
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function dynamic_const(C $c): void {
  echo $c::MY_CONSTANT;
}

// TEST-CHECK-BAL: define $root.cgets
// CHECK: define $root.cgets($this: *void) : *void {
// CHECK: #b0:
// CHECK: // .column 11
// CHECK:   n0 = __sil_lazy_class_initialize(<C>)
// CHECK: // .column 11
// CHECK:   n1 = $builtins.hack_field_get(n0, "prop3")
// CHECK: // .column 3
// CHECK:   n2 = $root.sink(null, n1)
// CHECK: // .column 2
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
// CHECK: // .column 6
// CHECK:   n1 = __sil_lazy_class_initialize(<C>)
// CHECK: // .column 15
// CHECK:   n2 = $root.source(null)
// CHECK: // .column 15
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK: // .column 15
// CHECK:   store &$1 <- n0: *HackMixed
// CHECK: // .column 15
// CHECK:   n3 = $builtins.hhbc_is_type_struct_c(n2, n0, $builtins.hack_int(1), $builtins.hack_int(0))
// CHECK: // .column 15
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 15
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK: // .column 15
// CHECK:   n4: *HackMixed = load &$0
// CHECK: // .column 15
// CHECK:   store &$1 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   store n1.?.prop3 <- n4: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: #b2:
// CHECK: // .column 15
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK: // .column 15
// CHECK:   n5: *HackMixed = load &$0
// CHECK: // .column 15
// CHECK:   n6: *HackMixed = load &$1
// CHECK: // .column 15
// CHECK:   n7 = $builtins.hhbc_throw_as_type_struct_exception(n5, n6)
// CHECK:   unreachable
// CHECK: }
function sets(): void {
  C::$prop3 = source() as float;
}

// TEST-CHECK-BAL: type TestFieldNamesWithTextualIdents
// CHECK: type TestFieldNamesWithTextualIdents = .kind="class" {
// CHECK:   n: .public *HackInt;
// CHECK:   mangled:::n0n: .public *HackInt;
// CHECK:   mangled:::n10: .public *HackInt
// CHECK: }
class TestFieldNamesWithTextualIdents {

  public function __construct(public int $n, public int $n0n, public int $n10) {
  }

}

// TEST-CHECK-BAL: type TestFieldNamesWithKeywordsAndConflicts
// CHECK: type TestFieldNamesWithKeywordsAndConflicts = .kind="class" {
// CHECK:   mangled:::define: .public *HackInt;
// CHECK:   define_: .public *HackFloat
// CHECK: }
class TestFieldNamesWithKeywordsAndConflicts {

  public function __construct(public int $define, public float $define_) {
  }

}

// TEST-CHECK-BAL: type TestClassWithGenerics
// CHECK: type TestClassWithGenerics = .kind="class" {
// CHECK:   t: .public *HackMixed;
// CHECK:   b: .public *Box
// CHECK: }
// TEST-CHECK-BAL: define TestClassWithGenerics.__construct
// CHECK: define TestClassWithGenerics.__construct($this: .notnull *TestClassWithGenerics, $t: .typevar="T" *HackMixed, $b: *Box) : *HackMixed {
// CHECK: #b0:
// CHECK: // .column 58
// CHECK:   n0: *HackMixed = load &$this
// CHECK: // .column 58
// CHECK:   n1 = $builtins.hhbc_check_this(n0)
// CHECK: // .column 58
// CHECK:   n2: *HackMixed = load &$b
// CHECK: // .column 58
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.b <- n2: *HackMixed
// CHECK: // .column 40
// CHECK:   n4: *HackMixed = load &$this
// CHECK: // .column 40
// CHECK:   n5 = $builtins.hhbc_check_this(n4)
// CHECK: // .column 40
// CHECK:   n6: *HackMixed = load &$t
// CHECK: // .column 40
// CHECK:   n7: *HackMixed = load &$this
// CHECK:   store n7.?.t <- n6: *HackMixed
// CHECK: // .column 4
// CHECK:   ret null
// CHECK: }
class Box<T> {}

class TestClassWithGenerics<T> {

  public function __construct(public T $t, public Box<T> $b) {
  }

}
