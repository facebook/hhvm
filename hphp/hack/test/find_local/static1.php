<?hh

function test($x) {  // Should match
  $y = $x;           // Should match
  static $x = 123;   // Should not match
  $z = $x;           // Should not match
}
