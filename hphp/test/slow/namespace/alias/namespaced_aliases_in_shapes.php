<?hh

namespace Foo;

class MyClass {
  const type MySubShape = shape('herp' => string);
}
type MyShape = shape('derp' => MyClass::MySubShape);


<<__EntryPoint>>
function main_namespaced_aliases_in_shapes() :mixed{
\var_dump(type_structure(MyShape::class));
}
