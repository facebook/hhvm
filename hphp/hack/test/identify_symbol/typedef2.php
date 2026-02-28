<?hh

class SomeClass {}
type SomeTypeAlias = SomeClass;

function test(
  SomeClass $c,
  SomeTypeAlias $d
) : void {}
