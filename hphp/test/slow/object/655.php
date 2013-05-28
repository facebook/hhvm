<?php

class g {
  public $v;
  function set($v) {
    $this->v = $v;
    return $this;
  }
}
function foo() {
  $z = 1;
  $qd = array('x' => $z);
  $a = G()->set($qd);
  var_dump($a);
  $qd['e'] = true;
  $b = G()->set($qd);
  var_dump($a);
}
function G() {
  return new g;
}
foo();
