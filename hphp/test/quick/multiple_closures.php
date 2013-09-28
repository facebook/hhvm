<?php

class foo {
  public function bar() {
    $z = function() {};
    $zz = function() {};
    $zzz = function() {};
    $zzzz = function() {};
    $zzzzz = function() {};
    $zzzzzz = function() {};
  }
}
function main() {
  $l = new foo;
  $l->bar();
}
main();
