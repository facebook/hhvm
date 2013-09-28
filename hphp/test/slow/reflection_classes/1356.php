<?php

$z=true;
if ($z) {
  class AaaA {
    function f() {
      var_dump(get_class());
    }
  }
}
 else {
  class aAAa {
}
}
class BbBb {
}
$r = new ReflectionClass('aaaa');
var_dump($r->getName());
$r = new ReflectionClass('bbbb');
var_dump($r->getName());
$a = new aaaa;
$a->f();
