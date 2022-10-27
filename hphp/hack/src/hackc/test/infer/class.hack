// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: type _CC$static = {
// CHECK: }

// CHECK: type _CC = {
// CHECK: }

// CHECK: global static_singleton::_CC

// CHECK: define _MC::24init_static() : void {
// CHECK: #b0:
// CHECK:   n0 = alloc_words(0)
// CHECK:   store &static_singleton::_CC <- n0: *_CC$static
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
