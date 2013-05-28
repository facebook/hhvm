<?php

function show_without_extra_vardump_nonsense($arr) {
  echo 'array (' . count($arr) . ") {\n";
  foreach ($arr as $val) {
    echo "  $val\n";
  }
  echo "}\n";
}
function do_wonderful_things_with($r) {
  echo "yall know what time it is. time to show you some properties\n";
  $props = array();
  foreach ($r->getProperties() as $prop) {
    $props[] = $prop->getName();
  }
  asort($props);
  show_without_extra_vardump_nonsense($props);
  $meths = array();
  echo "yall know what time it is now too. time to show you some methods\n";
  foreach ($r->getMethods() as $meth) {
    $meths[] = $meth->getName();
  }
  asort($meths);
  show_without_extra_vardump_nonsense($meths);
}

trait T {
  private $priv;
  protected $prot;
  public $pub;

  private static $priv_st;
  protected static $prot_st;
  public static $pub_st;

  private function fpriv() {
}
  protected function fprot() {
}
  public function fpub() {
}

  private static function fpriv_st() {
}
  protected static function fprot_st() {
}
  public static function fpub_st() {
}
}

trait U {
  public $foo;
  public static $static;

  public function ffoo() {
}
  public static function fstatic() {
}
}

class C {
  use T;

  private $c_priv;
}

class D extends C {
  use U;

  public $class_prop;
  public function class_method() {
}

}

$r = new ReflectionClass('C');
do_wonderful_things_with($r);

$r = new ReflectionClass('D');
do_wonderful_things_with($r);
