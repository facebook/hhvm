<?php

if (isset($g)) {
 class X {
}
 }
else {
  class X {
    function __destruct() {
      var_dump(__METHOD__);
    }
  }
}
class Y extends X {
  function __destruct() {
    var_dump(__METHOD__);
    parent::__destruct();
  }
}
class Z extends X {
}
function test($t) {
  var_dump('test:'.$t);
  new $t(1,2);
}
test('X');
test('Y');
test('Z');
