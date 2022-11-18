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

// CHECK: define C.$init_static() : void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.alloc_words(0)
// CHECK:   store &static_singleton::C <- n0: *C$static
// CHECK:   ret 0
// CHECK: }

<<__ConsistentConstruct>>
class C {
  public int $prop1 = 42;
  public string $prop2 = "hello";
  public static float $prop3 = 3.14;
  public static mixed $prop4 = null;

  // Test reserved token.
  public int $type = 2;

  public function __construct(int $a, int $b, int $c) {
  }

  // CHECK: define C.cons_static(this: *C) : *HackMixed {
  // ...
  // CHECK:   n2: *C = load &this
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

  // CHECK: define C.cons_self(this: *C) : *HackMixed {
  // ...
  // CHECK:   n2: *C$static = load &static_singleton::C
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

  // CHECK: define C.cons_inst(this: *C) : *HackMixed {
  // ...
  // CHECK:   n0: *C$static = load &static_singleton::C
  // CHECK:   n1 = $builtins.lazy_initialize(n0)
  // ...
  // CHECK:   n2: *C$static = load &static_singleton::C
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

  // CHECK: define C.static_signature(this: *C$static, $a: *HackMixed, $b: *HackMixed) : *HackMixed {
  // Lots of stuff elided here...
  // CHECK: }
  public static function static_signature(mixed $a, mixed $b): void {
    if ($a == $b) {
      echo "equal";
    } else {
      echo "unequal";
    }
  }
}

// CHECK: global static_singleton::C : *C$static
