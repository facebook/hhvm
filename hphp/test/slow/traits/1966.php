<?php

trait T1 {
  var $x = 1977;
}
trait T2 {
  var $x = 1977;
}
class MY_CLASS {
  use T1, T2;
  var $abc = 1;
  public function printProps() {
    var_dump($this->x);
  }
}
$o = new MY_CLASS;
$o->printProps();
?>
