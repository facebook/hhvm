<?php

trait T1 {
  public $x = 3;
}
trait T2 {
  use T1;
}
trait T3 {
  use T1;
}
class C {
  use T2, T3;
  public function printProps() {
    var_dump($this->x);
  }
}
$o = new C;
$o->printProps();
?>
