<?hh

namespace foo\baz {
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace bar\baz {
  function foo() {
    \var_dump(__NAMESPACE__);
  }
}
namespace bar {
  use foo\baz as baz;
  <<__EntryPoint>> function main(): void {
    baz\foo();
  }
}
