<?hh

namespace A\B;
class B {
  static $baz = 'baz';
  const FOO = 30;
  function f() {
    return Foo::NoSuchConstant;
  }
}
$x = 1;
