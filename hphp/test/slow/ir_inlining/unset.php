<?php

class NonExistProp {
  private $x;
  public function __construct() {
 $x = "str";
 }
  public function unsetIt() {
    unset($this->x);
  }
  public function getX() {
    return $this->x;
  }
}

// TODO: this will need a hopt to enable throw
function thrower() {
  //var_dump(debug_backtrace());
  throw new Exception("Yo");
}

function test7() {
  set_error_handler('thrower');
  try {
    $obj = new NonExistProp();
    $obj->unsetIt();
    $k = new Dtor();
    echo $obj->getX();
    echo "\n";
  }
 catch (Exception $x) {
}
}

test7();
