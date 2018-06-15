<?hh  //strict
// comment
function a_rename(): int {
  return b_rename();
}

function b_rename(): int {
  return 42;
}

class TestClass {
  public function __construct(int $i) {}

  public function test_method(): int {
    return 1;
  }
}

function test_rename_class_method(): void {
  $test_class = new TestClass(1);
  $test_class->test_method(); $test_class->test_method();
}

function test_rename_localvar(int $x): void { //  Should match
  $x = 3;                                     //  Should match
  j($x) + $x + h("\$x = $x");                 //  1st, 2nd, and 4th should match
  $lambda1 = $x ==> $x + 1;                   //  Should not match
  $lambda2 = $a ==> $x + $a; // Renaming this $x    //  Should match
  $lambda3 = function($x) {                   //  Should not match
    return $x + 1; };                         //  Should not match
  $lambda4 = function($b) use($x) {           //  Should match
    return $x + $b; };                        //  Should match
}
