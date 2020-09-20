<?hh

namespace Foo {
  trait T {
    public static function f(): void {
      \var_dump(__CLASS__);
      \var_dump(__TRAIT__);
      \var_dump(__METHOD__);
    }
  }
  class C {
    use T;
  }
  <<__EntryPoint>>
  function main(): void {
    \var_dump(__LINE__);
    \var_dump(__FILE__);
    \var_dump(__DIR__);
    \var_dump(__FUNCTION__);
    \var_dump(__NAMESPACE__);
    \var_dump(__COMPILER_FRONTEND__);
    C::f();
  }
}
