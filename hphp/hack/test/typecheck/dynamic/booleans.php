<?hh // strict

function testBools(dynamic $x): void {
  $y = $x && true; // $y : bool
  hh_show($y);
  $y = $x || false; // $y : bool
  hh_show($y);
  $y = !$x; // $y : bool
  hh_show($y);
  $y = $x === 5; // $y : bool
  hh_show($y);
  $y = $x == 5; // $y : bool
  hh_show($y);
  if ($x) // valid, no sketchy null check warning
  {
  }
}
