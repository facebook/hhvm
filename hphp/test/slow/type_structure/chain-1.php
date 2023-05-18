<?hh

type A = int;
type B = A;
type C = B;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure('C'));
}
