<?php

if (isset($g)) {
 class X {
}
 }
else {
  class X {
    function foo() {
      var_dump(__METHOD__);
    }
  }
}
class Y extends X {
  function foo() {
    var_dump(__METHOD__);
    parent::foo();
  }
}
class Z extends X {
}
function test($t) {
  var_dump('test:'.$t);
  (new $t(1,2))->foo();
}
test('X');
test('Y');
test('Z');
