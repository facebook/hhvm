<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

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
  public function test() {
    $var = 4;
    $this->x =& $var;
    $this->printX();
    $var = 5;
    $this->printX();
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

