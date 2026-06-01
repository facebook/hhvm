<?hh

class Bar {}

class Foo {
  const type T = Bar;
}

function test(): class<Bar> {
  return HH\type_structure_class(Foo::class, 'T');
}
