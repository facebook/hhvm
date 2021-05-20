<?hh

abstract class A {
  abstract const type T = int;
}
interface I {
  const type T = string;
}
class C extends A implements I {}

<<__EntryPoint>>
function main(): void {
  // expecting TypeStructureKind::OF_STRING = 4
  var_dump(type_structure(C::class, "T"));
}
