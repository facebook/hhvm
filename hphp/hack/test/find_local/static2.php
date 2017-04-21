<?hh

function test($x) {  // Should not match
  $y = $x;           // Should not match
  static $x = 123;   // Should match
  $z = $x;           // Should match
}
