<?hh

class MyClass {
  const ASD = 'wat';
}
type Foo = MyClass;

function test(): void {
  // error: making a new type name doesn't make it available outside
  // of type expressions (parameter lists, etc).
  $x = Foo::ASD;
}
test();
