<?hh

abstract class A {
  abstract const type T = int;
}
trait Tr {
  const type T = string;
}
class C extends A {
  use Tr;
}

<<__EntryPoint>>
function main(): void {
  // expecting TypeStructureKind::OF_STRING = 4
  var_dump(type_structure(C::class, "T"));
}
