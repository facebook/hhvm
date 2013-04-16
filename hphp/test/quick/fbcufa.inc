<?php
echo "_fbcufa_init.php begin\n";
function doThrow() {
  throw new Exception("blah");
}
function foo($x, $y) {
  var_dump($x, $y);
  return 42;
}
class C {
  public $blah = 123;
  public static function bar($x) {
    return $x * 3;
  }
  public function baz($x) {
    return $x + $this->blah;
  }
}
echo "_fbcufa_init.php end\n";
