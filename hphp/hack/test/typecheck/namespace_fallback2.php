<?hh // strict

namespace Foo {
  class :my-xhp {}

  function test(): :my-xhp {
    return <my-xhp />;
  }
}
