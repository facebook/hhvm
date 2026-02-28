<?hh

// Checking the type of the ::class implcit constant.
// See functions class_class_decl in OCaml or decl_class_class in Rust
class C {}

function f(): void {
  hh_show(C::class);
}
