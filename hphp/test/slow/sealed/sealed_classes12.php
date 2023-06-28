<?hh

namespace SealedClass12;

<<__Sealed(MyClass::class)>>
class SomeClass { const FOO = 42; }

<<__EntryPoint>>
function main_sealed_classes12() :mixed{
  var_dump(Class2::FOO);
}
