// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: type C$static = {
// CHECK: }

// CHECK: type C = {
// CHECK: }

// CHECK: global static_singleton::C

// CHECK: define C.$init_static() : void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.alloc_words(0)
// CHECK:   store &static_singleton::C <- n0: *C$static
// CHECK:   ret 0

class C {
  public static function cmp(mixed $a, mixed $b): void {
    if ($a == $b) {
      echo "equal";
    } else {
      echo "unequal";
    }
  }
}
