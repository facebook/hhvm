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
    return $this->x++;
  }
}


function foo($o){
  echo $o->getX();
  echo "\n";
  echo $o->incX();
  echo "\n";
  echo $o->getX();
  echo "\n";
}

$c = new C(2); // change to 4.0
foo($c);

