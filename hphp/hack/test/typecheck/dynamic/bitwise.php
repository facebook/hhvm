<?hh // strict

function testBitwise(dynamic $x): void {
  $y = $x & 5; // $y : num
  hh_show($y);
  $y = $x | 5; // $y : num
  hh_show($y);
  $y = $x ^ 5; // $y : num
  hh_show($y);
  $y = $x >> 5; // $y : num
  hh_show($y);
  $y = $x << 5; // $y : num
  hh_show($y);
  $y = $x ^ $x; // $y : dynamic
  hh_show($y);
  $y = ~$x; // $y : dynamic
  hh_show($y);
}
