<?hh

type A<Ta> = int;
type AA<Taa> = A<float>;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure_for_alias(nameof AA));
}
