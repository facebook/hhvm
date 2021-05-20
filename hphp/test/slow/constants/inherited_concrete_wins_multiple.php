<?hh

interface I1 {
  abstract const type T = int;
}
interface I2 {
  const type T = bool;
}
interface I3 {
  abstract const type T = string;
}

class C implements I1, I2, I3 {}

<<__EntryPoint>>
function main(): void {
  // expecting TypeStructureKind::OF_BOOL = 2
  var_dump(type_structure(C::class, "T"));
}
