<?hh // strict

function test(int $x): void {
  /* HH_FIXME[2087] */
  $f = function() use (&$x) {};
}
