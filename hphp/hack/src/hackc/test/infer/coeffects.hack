// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define .static_coeffects = "pure" $root.fpure
// CHECK: define .static_coeffects = "pure" $root.fpure($this: *void) : *void {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }
function fpure()[] : void {}

// TEST-CHECK-BAL: define .static_coeffects = "write_props" $root.fwrite_props
// CHECK: define .static_coeffects = "write_props" $root.fwrite_props($this: *void) : *void {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }
function fwrite_props()[write_props] : void {}

// TEST-CHECK-BAL: define .static_coeffects = "defaults" $root.fdefaults
// CHECK: define .static_coeffects = "defaults" $root.fdefaults($this: *void) : *void {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }
function fdefaults()[defaults] : void {}

// TEST-CHECK-BAL: define .static_coeffects = "write_props", "globals" $root.foo
// CHECK: define .static_coeffects = "write_props", "globals" $root.foo($this: *void) : *void {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }
function foo()[write_props, globals]: void {}
