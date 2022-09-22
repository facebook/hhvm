// RUN: %hackc compile-infer %s | FileCheck %s
// CHECK: attribute source_language = "hack"

// CHECK: define _Hmain(params: HackParams) : *Mixed {
// CHECK:  hhbc_print(hack_string("Hello, World!\n"))
// CHECK:  ret hack_null()

<<__EntryPoint>>
function main(): void {
  echo "Hello, World!\n";
}
