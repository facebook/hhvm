<?hh // strict

abstract xhp class foo extends XHPTest {}

xhp class bar extends foo {}

function baz(): void {
  <bar/>;
}
