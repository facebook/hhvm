<?hh // strict

namespace {
  class :my-xhp {}
}

namespace Foo {
  function test(): :my-xhp {
    return <my-xhp />;
  }
}
