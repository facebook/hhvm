<?hh

namespace A\B;
class Foo {
  public static $baz = 32;
  function __construct(varray $a) {
    \var_dump($a);
  }
  function callUnknownClassMethod($method) :mixed{
    return SomeUnknownClass::$method();
  }
  function unsetStaticProperty() :mixed{
    unset(Foo::$baz);
  }
}

<<__EntryPoint>>
function main_2227() :mixed{
  if (\rand(0, 1)) {
    include '2227-1.inc';
  } else {
    include '2227-2.inc';
  }

  $f = new Foo(vec[0]);
  \var_dump(Foo::$baz);
  \var_dump(B::FOO);
  \var_dump(B::$baz);
}
