<?hh

function testAssign(dynamic $x, dynamic $y): void {
  $x += 5; // $x is now a num
  hh_show($x);
  $y .= "string"; // $y is now a string
  hh_show($y);
}
