<?hh

class Foo {
  const type T = int;
}

function test(): void {
  HH\type_structure_class(Foo::class, 'T');
}
