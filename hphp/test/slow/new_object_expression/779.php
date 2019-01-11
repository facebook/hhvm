<?php

class X {
  function __construct($a, &$b) {
  }
}
function test($a) {
  $b = 1;
  return new X($a, &$b);
}

<<__EntryPoint>>
function main_779() {
var_dump(test(3));
}
