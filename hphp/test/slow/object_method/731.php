<?php

interface intf {
  function meth();
}
class m {
  function meth() {
    return 1;
  }
}
class m2 extends m implements intf {
}
class m3 extends m2 {
  function f() {
    var_dump(parent::meth());
  }
}
$y = new m3;
$y->f();
function g() {
  $y = new m3;
  var_dump($y->meth());
}
g();
