<?hh

class Foo {
  const type T = int;
}

function test(): void {
  HH\type_structure_classname(Foo::class, 'T');
}
