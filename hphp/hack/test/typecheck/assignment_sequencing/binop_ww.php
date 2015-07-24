<?hh // strict

function x(int $y): string {
  $y = ($x = 'lol').($x = 'wut');
  return $x;
}
