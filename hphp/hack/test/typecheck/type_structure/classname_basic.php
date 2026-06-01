<?hh

class Bar {}

class Foo {
  const type T = Bar;
}

function test(): classname<Bar> {
  return HH\type_structure_classname(Foo::class, 'T');
}
