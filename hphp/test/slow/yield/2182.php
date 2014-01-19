<?php

class X {
  private $a = array(1,2,3);
  function foo() {
 yield $this->a;
 }
}
if (isset($g)) {
  class Y {
}
}
 else {
  class Y extends X {
}
}
class Z extends Y {
}
function test() {
  $z = new Z;
  foreach ($z->foo() as $v) {
    var_dump($v);
  }
}
test();
