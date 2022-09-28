<?hh // strict

type MyAlias1 = shape(
  'a' => int,
  ?'b' => string,
);

class C {
  const type T = int;
}

function test(): void {
  HH\type_structure_for_alias(MyAlias1::class);
  type_structure_for_alias(MyAlias1::class);
}
