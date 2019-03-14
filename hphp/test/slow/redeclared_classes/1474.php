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
  include '1474-1.inc';
}
 else {
  include '1474-2.inc';
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
