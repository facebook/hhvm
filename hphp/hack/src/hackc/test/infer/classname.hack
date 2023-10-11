// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.test
// CHECK: define $root.test($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = __sil_lazy_class_initialize(<Tools>)
// CHECK:   n1 = Tools$static.call_test(n0, __sil_get_lazy_class(<Circle>))
// CHECK:   ret null
// CHECK: }
function test(): void {
  Tools::call_test(Circle::class);
}
