<?php

trait MY_TRAIT {
  public $x;
}
class MY_CLASS{
  use MY_TRAIT;
  public $x;
  public function printX() {
    var_dump($this->x);
    $this->x = 10;
    var_dump($this->x);
  }
}
$o = new MY_CLASS;
$o->printX();
?>
