// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: attribute source_language = "hack"

// CHECK: define _Hmain(params: HackParams) : *Mixed {
// CHECK-DAG:  [[V0:n[0-9]+]] = hack_string("Hello, World!\n")
// CHECK-DAG:  hhbc_print([[V0]])
// CHECK-DAG:  [[V1:n[0-9]+]] = hack_null()
// CHECK:  ret [[V1]]

<<__EntryPoint>>
function main(): void {
  echo "Hello, World!\n";
}
