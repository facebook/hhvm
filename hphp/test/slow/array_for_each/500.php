<?php

class X {
}
function test() {
  $a = array(new X, 0);
  foreach ($a as $v) {
    var_dump($v);
  }
  $a = null;
  var_dump('done');
}

<<__EntryPoint>>
function main_500() {
test();
var_dump('exit');
}
