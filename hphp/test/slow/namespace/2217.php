<?hh

namespace foo\baz {
  function foo() :mixed{
    \var_dump(__NAMESPACE__);
  }
}
namespace bar\baz {
  function foo() :mixed{
    \var_dump(__NAMESPACE__);
  }
}
namespace {
  use foo\baz as baz;
  <<__EntryPoint>> function main(): void {
    baz\foo();
  }
}
