<?hh // strict

function test(int $x): void {
  /* HH_FIXME[2095] */
  $f = function() use (&$x) {};
}
