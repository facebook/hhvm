<?hh

namespace SealedClass13;

<<__Sealed(SomeInterface::class)>>
interface MyInterface { const FOO = 42; }

<<__EntryPoint>>
function main_sealed_classes13() :mixed{
  var_dump(SomeInterface2::FOO);
}
