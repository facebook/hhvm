// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: type C$static = {
// CHECK:   prop3: .public *HackFloat;
// CHECK:   prop4: .public *HackMixed
// CHECK: }

// CHECK: type C = {
// CHECK:   prop1: .public *HackInt;
// CHECK:   prop2: .public *HackString;
// CHECK:   type_: .public *HackInt
// CHECK: }

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

  // CHECK: define C.cons_static($this: *C) : *HackMixed {
  // ...
  // CHECK:   n2: *C = load &$this
  // CHECK:   n3 = $builtins.hack_get_static_class(n2)
  // CHECK:   n4 = n3.HackMixed.__factory()
  // CHECK:   store &$2 <- n4: *HackMixed
  // ...
  // CHECK:   n27: *HackMixed = load &$2
  // ...
  // CHECK:   n30 = n27.HackMixed.__construct($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
  // ...
  // CHECK: }
  public function cons_static(): void {
    $a = new static(1, 2, 3);
  }

  // CHECK: define C.cons_self($this: *C) : *HackMixed {
  // ...
  // CHECK:   n2: *C$static = load &C$static::static_singleton
  // CHECK:   n3 = $builtins.lazy_initialize(n2)
  // CHECK:   n4 = n2.HackMixed.__factory()
  // CHECK:   store &$2 <- n4: *HackMixed
  // ...
  // CHECK:   n14: *HackMixed = load &$2
  // ...
  // CHECK:   n17 = n14.HackMixed.__construct($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
  // ...
  // CHECK: }
  public function cons_self(): void {
    $a = new self(1, 2, 3);
  }

  // CHECK: define C.cons_inst($this: *C) : *HackMixed {
  // ...
  // CHECK:   n0: *C$static = load &C$static::static_singleton
  // CHECK:   n1 = $builtins.lazy_initialize(n0)
  // ...
  // CHECK:   n2: *C$static = load &C$static::static_singleton
  // CHECK:   n3 = $builtins.lazy_initialize(n2)
  // CHECK:   n4 = n2.HackMixed.__factory()
  // CHECK:   store &$2 <- n4: *HackMixed
  // ...
  // CHECK:   n14: *HackMixed = load &$2
  // ...
  // CHECK:   n17 = n14.HackMixed.__construct($builtins.hack_int(1), $builtins.hack_int(2), $builtins.hack_int(3))
  // ...
  // CHECK: }
  public function cons_inst(): void {
    $a = new C(1, 2, 3);
  }

  // CHECK: define C$static.static_signature($this: *C$static, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
  // Lots of stuff elided here...
  // CHECK: }
  //
  // CHECK: define C.static_signature($this: *C, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
  // CHECK-NEXT: #b0:
  // CHECK-NEXT: // forward to the static method
  // CHECK-NEXT:   n0: *C = load &$this
  // CHECK-NEXT:   n1 = $builtins.hack_get_static_class(n0)
  // CHECK-NEXT:   n2: *HackMixed = load &$a
  // CHECK-NEXT:   n3: *HackMixed = load &$b
  // CHECK-NEXT:   n4 = C$static.static_signature(n1, n2, n3)
  // CHECK-NEXT:   ret n4
  // CHECK-NEXT: }
  public static function static_signature(mixed $a, mixed $b): void {
    if ($a == $b) {
      echo "equal";
    } else {
      echo "unequal";
    }
  }

  // CHECK: define C.test_const($this: *C) : *HackMixed {
  // CHECK: local $x: *void, base: *HackMixed
  // CHECK: #b0:
  // CHECK:   n0: *HackMixed = load &C$static::MY_CONSTANT
  // CHECK:   store &$x <- n0: *HackMixed
  // CHECK:   ret $builtins.hack_null()
  // CHECK: }
  public function test_const(): void {
    $x = C::MY_CONSTANT;
  }
}

// CHECK: global C$static::MY_CONSTANT : *HackMixed
// CHECK: global C$static::static_singleton : *C$static
