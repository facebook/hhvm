<?hh

<<__Sealed(MyClass::class)>>
class SomeClass { const FOO = 42; }

function __autoload($x) {
  require_once "sealed_classes12.inc";
}


<<__EntryPoint>>
function main_sealed_classes12() {
var_dump(Class2::FOO);
}
