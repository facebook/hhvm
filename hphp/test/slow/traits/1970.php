<?php

class MY_BASE {
  public $x = 3;
}
trait MY_TRAIT {
  public $x = 4;
}
class MY_CLASS extends MY_BASE {
  use MY_TRAIT;
  public $x = 4;
  public function printX() {
    var_dump($this->x);
  }
}
$o = new MY_CLASS;
$o->printX();
?>
