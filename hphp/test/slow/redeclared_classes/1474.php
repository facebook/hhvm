<?php

function nop($en,$es){
}
;
set_error_handler('nop');
class X {
  function bar() {
    var_dump($this);
  }
}
if (1) {
  class U {
  }
}
 else {
  class U extends X {
  }
}

class V extends U {
}

function test() {
  $x = new X;
  $x->bar();
  $x = new V;
  $x->bar();
}
test();
