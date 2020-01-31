<?hh // strict

abstract xhp class foo {}

xhp class bar extends foo {}

function baz(): void {
  <bar/>;
}
