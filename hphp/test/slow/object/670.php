<?php

$x = null;
$y = 0;
class C {
  public $p = "C::\$p";
  function __destruct() {
    global $x, $y;
    print "In C::__destruct(): $y
";
    $this->p = "changed";
    $x = $this;
  }
}
$c = new C();
var_dump($c);
$c = null;
$y = 140;
var_dump($x);
$x = null;
