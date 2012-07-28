<?php
class B {
  private $y=2;
}
class C extends B {
  public $w=3;
  function dump_vars() {
    var_dump(get_class_vars('C'));
  }
}
$o = new C;
$o->dump_vars();
