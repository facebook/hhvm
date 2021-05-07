<?hh

abstract class A {
  abstract const type T;
}
interface I {
  abstract const type T = int;
}

class C extends A implements I {}

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure(C::class, "T"));
}
