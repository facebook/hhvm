<?hh

namespace Foo;

class MyClass {
  const type MySubShape = shape('herp' => string);
}
type MyShape = shape('derp' => MyClass::MySubShape);

var_dump(type_structure(MyShape::class));
