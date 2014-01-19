<?php

abstract class c {
  public static $arr = array();
  function g() {
    $cl = new ReflectionClass(get_class($this));
    $p = $cl->getProperty('arr');
    return $p->getValue();
  }
}
abstract class aa extends c {
  public function get_arr() {
    $actions = parent::get_arr();
    return $actions;
  }
}
class a extends aa {
  public static $arr = array('v');
}
$x = new a;
var_dump($x->g());
