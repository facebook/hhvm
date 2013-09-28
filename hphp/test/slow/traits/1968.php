<?php

class MY_BASE {
  static public $x = 3;
}
trait MY_TRAIT {
  static public $x = 3;
}
class MY_CLASS extends MY_BASE {
  use MY_TRAIT;
  public function printX() {
    var_dump(self::$x);
    self::$x = 4;
  }
}
$o = new MY_CLASS;
$o->printX();
var_dump(MY_BASE::$x);
var_dump(MY_CLASS::$x);
?>
