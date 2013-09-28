<?php

class X {
  function __destruct() {
 var_dump(__METHOD__);
 }
}
function test() {
  $a = array(new X, 0);
  foreach ($a as $v) {
    var_dump($v);
  }
  $a = null;
  var_dump('done');
}
test();
var_dump('exit');
