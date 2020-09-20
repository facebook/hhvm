<?hh // strict

function testString(dynamic $x): void {
  $y = $x."hello"; // $y : string
  hh_show($y);
  $y = $x.$x; // $y : string
  hh_show($y);
}
