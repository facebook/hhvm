<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

class C {
  public $x;
  public function __construct($val) {
    $this->x = $val;
  }
  public function getX() {
    return $this->x;
  }
  public function incX() {
    return ++$this->x;
  }
  public function setX($val) {
    $this->x = $val;
  }
  public function printX() {
    echo "x is ";
    echo $this->x;
    echo "\n";
  }
  public function test2(&$var, $val) {
    $var = $val;
    $this->printX();
  }
  public function test() {
    $this->test2(&$this->x, 4);
    $this->test2(&$this->x, 5);
  }
}


function foo($o){
  echo $o->getX();
  echo "\n";

  echo $o->incX();
  echo "\n";
}

$var = 3; // change to 4.0
$c = new C($var);
$c->test();
foo($c);
echo "var is $var\n";
$var = 4;
$c->setX(3);
echo "var is $var\n";

