<?hh

namespace A\B;
class B {
  public static $baz = 'baz';
  const FOO = 30;
  function f() {
    return Foo::NoSuchConstant;
  }
}
