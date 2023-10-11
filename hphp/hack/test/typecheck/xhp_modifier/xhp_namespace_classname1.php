<?hh

namespace {
  xhp class foo:bar extends XHPTest {}

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
