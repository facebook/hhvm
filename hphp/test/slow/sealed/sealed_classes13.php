<?hh

<<__Sealed(SomeInterface::class)>>
interface MyInterface { const FOO = 42; }

var_dump(SomeInterface2::FOO);

function __autoload($x) {
  require_once "sealed_classes13.inc";
}
