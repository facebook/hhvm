<?hh

<<__Sealed(MyClass::class)>>
class SomeClass { const FOO = 42; }

var_dump(Class2::FOO);

function __autoload($x) {
  require_once "sealed_classes12.inc";
}
