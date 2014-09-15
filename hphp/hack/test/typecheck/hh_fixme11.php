<?hh // strict

function test(int $x): void {
  /* HH_FIXME[1002] */
  $f = function() use (&$x) {};
}
