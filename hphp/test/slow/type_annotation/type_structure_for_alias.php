<?hh

type MyAlias1 = shape(
  'a' => int,
  ?'b' => string,
);

class C {
  const type T = int;
}


<<__EntryPoint>>
function main_type_structure_for_alias() {
  var_dump(HH\type_structure_for_alias(MyAlias1::class));
}
