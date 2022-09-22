// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: attribute source_language = "hack"

// CHECK: define _Hmain(params: HackParams) : *Mixed {
// CHECK-DAG:  [[V0:n[0-9]+]] = copy(hack_string("Hello, World!\n"))
// CHECK-DAG:  hack_print([[V0]])
// CHECK-DAG:  [[V1:n[0-9]+]] = hack_null()
// CHECK:  ret [[V1]]

<<__EntryPoint>>
function main(): void {
  echo "Hello, World!\n";
}
