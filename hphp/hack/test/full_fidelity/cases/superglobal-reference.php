<?hh // partial

function f() {
  x(&$_SERVER['foo']);
  y(&$GLOBALS['bar']);
}
