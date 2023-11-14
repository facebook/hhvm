// RUN: %hackc compile-infer --hide-static-coeffects --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type X$static
// CHECK: type X$static = .kind="class" .static {
// CHECK:   T: .public .type_constant *HackMixed
// CHECK: }
class X {
    const type T = int;
}

// TEST-CHECK-BAL: type C$static
// CHECK: type C$static = .kind="class" .static {
// CHECK:   T: .public .type_constant= "X"  *HackMixed;
// CHECK:   TS: .public .type_constant= "X::T"  *HackMixed
// CHECK: }
class C {
    const type T = X;
    const type TS = X::T;
}
