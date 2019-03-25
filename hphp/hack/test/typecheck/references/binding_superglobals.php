<?hh // partial

function f(): void {
  $x = null;
  $_GET = &$x;
}
