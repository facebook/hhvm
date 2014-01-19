<?php

print "Test begin\n";

$x = null;

class C {
  public $p = "C::\$p";
  function __destruct() {
    global $x;
    print "In C::__destruct()\n";
    $this->p = "changed";
    $x = $this;
  }
}

$c = new C();
var_dump($c);
$c = null;
# XXX The translator fails to notice that $x has changed, so break the basic
# block here to force the translator to re-read $x.
if (isset($g)) {}
var_dump($x);
$x = null;

print "Test end\n";
