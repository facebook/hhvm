<?hh

namespace A\B;
class Foo {
  static $baz = 32;
  function __construct(array $a) {
    \var_dump($a);
  }
  function callUnknownClassMethod($method) {
    return SomeUnknownClass::$method();
  }
  function unsetStaticProperty() {
    unset(Foo::$baz);
  }
}

<<__EntryPoint>>
function main_2227() {
  if (\rand(0, 1)) {
    include '2227-1.inc';
  } else {
    include '2227-2.inc';
  }

  $f = new Foo(varray[0]);
  \var_dump(Foo::$baz);
  \var_dump(B::FOO);
  \var_dump(B::$baz);
}
