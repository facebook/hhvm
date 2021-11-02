<?hh //partial

class C {
  private function ref_test_1 ($x) {  // Should match
    $y = $x;           // Should not match
    //static $x = 123;
    $z = $x;           // Request ID 11: Looking for this $x // Should match
  }

  public function ref_test_method(): int {
    return 0;
  }
}

interface IRefTestRequireExtends {
  require extends C;
}

function ref_test_require_extends(IRefTestRequireExtends $req_extends_test) {
  $req_extends_test->ref_test_method(); // Request ID 12
}

function j(int $x): int {
  return $x;
}

function h(string $x): int {
  return 5;
}

enum RefTestEnum: int {  // Request ID 14 on RefTestEnum
  SMALL = 0;
  MEDIUM = 1;
  LARGE = 2;
}

function ref_test_2(int $x): void {   //  Should match
  $x = 3; // Request ID 13: Looking for this $x   //  Should match
  j($x) + $x + h("\$x = $x");         //  First, second, and fourth should match
  $lambda1 = $x ==> $x + 1;           //  Should not match
  $lambda2 = $a ==> $x + $a;          //  Should match
  $lambda3 = function($x) {           //  Should not match
    return $x + 1; };                 //  Should not match
  $lambda4 = function($b) use($x) {   //  Should match
    return $x + $b; };                //  Should match
  $size = RefTestEnum::SMALL;
  $shape = shape(RefTestEnum::SMALL => 123);
}
