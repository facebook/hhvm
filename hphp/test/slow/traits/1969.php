<?php

trait MY_TRAIT {
  static public $x = 3;
}
class MY_CLASS{
  use MY_TRAIT;
  public function printX() {
    var_dump(self::$x);
  }
}
$o = new MY_CLASS;
$o->printX();
?>
