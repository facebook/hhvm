// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: type static::_CC = {
// CHECK: }

// CHECK: type _CC = {
// CHECK: }

// CHECK: global static_singleton::_CC

// CHECK: define get_static::_CC() : *static::_CC
// CHECK:   n0: *static::_CC = load &static_singleton::_CC
// CHECK:   n1 = raw_ptr_is_null(n0)
// CHECK:   jmp b2, b1
// CHECK:   #b2:
// CHECK:   prune ! n1
// CHECK:   ret n0
// CHECK:   #b1:
// CHECK:   prune n1
// CHECK:   n2 = alloc_words(0)
// CHECK:   store &static_singleton::_CC <- n2: *static::_CC
// CHECK:   ret n2

class C {
  public static function cmp(mixed $a, mixed $b): void {
    if ($a == $b) {
      echo "equal";
    } else {
      echo "unequal";
    }
  }
}
