<?hh

type MyShape = shape(
  'a' => ?int,
  'b' => bool,
  ?'c' => int,
);

function test(TypeStructure<MyShape> $type_structure): void {
  hh_show($type_structure['kind']);
  hh_show($type_structure['alias']);
  hh_show($type_structure['name']);
  hh_show($type_structure['fields']);
}
