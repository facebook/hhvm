<?hh

function testBitwise(dynamic $x): void {
  $y = $x & 5; // $y : int
  hh_show($y);
  $y = $x | 5; // $y : int
  hh_show($y);
  $y = $x ^ 5; // $y : int
  hh_show($y);
  $y = $x >> 5; // $y : int
  hh_show($y);
  $y = $x << 5; // $y : int
  hh_show($y);
  $y = $x ^ $x; // $y : ~int
  hh_show($y);
  $y = ~$x; // $y : ~int
  hh_show($y);
}
