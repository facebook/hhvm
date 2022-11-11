// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: type C$static = {
// CHECK:   prop3: .public *HackFloat;
// CHECK:   prop4: .public *HackMixed
// CHECK: }

// CHECK: type C = {
// CHECK:   prop1: .public *HackInt;
// CHECK:   prop2: .public *HackString
// CHECK: }

// CHECK: global static_singleton::C : *C$static

// CHECK: define C.$init_static() : void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.alloc_words(0)
// CHECK:   store &static_singleton::C <- n0: *C$static
// CHECK:   ret 0

class C {
  public int $prop1 = 42;
  public string $prop2 = "hello";
  public static float $prop3 = 3.14;
  public static mixed $prop4 = null;

  public static function cmp(mixed $a, mixed $b): void {
    if ($a == $b) {
      echo "equal";
    } else {
      echo "unequal";
    }
  }
}
