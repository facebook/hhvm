<?php
class A {
  public $one = 1;

  public function show_one() {
    echo $this->one;
  }
}
$a = new A;

var_dump(yaml_emit(array('a' => $a)));
?>
