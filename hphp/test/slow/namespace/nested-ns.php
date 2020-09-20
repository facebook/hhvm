<?hh
namespace Foo {

class Biz {
  function wat() {
    \var_dump(\get_class());
  }
}

namespace Bar {

class Baz {
  function wat() {
    \var_dump(\get_class());
  }
}

} /* Bar */

class Fiz {
  function wat() {
    \var_dump(\get_class());
  }
}

} /* Foo */

namespace {
<<__EntryPoint>> function main(): void {
$biz = new Foo\Biz();
$baz = new Foo\Bar\Baz();
$fiz = new Foo\Fiz();
$biz->wat();
$baz->wat();
$fiz->wat();
}
}
