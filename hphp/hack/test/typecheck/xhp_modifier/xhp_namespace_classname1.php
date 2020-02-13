<?hh // strict

namespace {
  xhp class foo:bar {}

  function usage_same(): void {
    <foo:bar/>;
    <:foo:bar/>;
  }

}

namespace foo {
  function fn(): void {
    <bar/>;
  }
}
