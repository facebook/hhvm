<?hh

<<__Sealed(SomeInterface::class)>>
interface MyInterface { const FOO = 42; }

function __autoload($x) {
  require_once "sealed_classes13.inc";
}


<<__EntryPoint>>
function main_sealed_classes13() {
var_dump(SomeInterface2::FOO);
}
