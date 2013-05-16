<?php

class Obj {
  public $val;
  public function __construct() {
    $this->val = "string";
  }
}

class CGetM {
  private $x;

  public function __construct() {
    $this->x = array(new Obj);
  }

  public function getVal() {
    return $this->x[0]->val;
  }
}

function main(CGetM $k) {
  return $k->getVal();
}

main(new CGetM());
