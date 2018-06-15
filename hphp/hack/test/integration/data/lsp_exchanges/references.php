<?hh

class C {
  private function ref_test_1 ($x) {  // Should not match
    $y = $x;           // Should not match
    static $x = 123;   // Should match
    $z = $x;           // Request ID 11: Looking for this $x // Should match
  }
}

function j(int $x): int {
  return $x;
}

function h(string $x): int {
  return 5;
}

function ref_test_2(int $x): void {   //  Should match
  $x = 3; // Request ID 12: Looking for this $x   //  Should match
  j($x) + $x + h("\$x = $x");         //  First, second, and fourth should match
  $lambda1 = $x ==> $x + 1;           //  Should not match
  $lambda2 = $a ==> $x + $a;          //  Should match
  $lambda3 = function($x) {           //  Should not match
    return $x + 1; };                 //  Should not match
  $lambda4 = function($b) use($x) {   //  Should match
    return $x + $b; };                //  Should match
}
