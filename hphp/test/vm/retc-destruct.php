<?php

function bt() {
  global $g;
  $g = debug_backtrace();
}

class X {
  function __destruct() { bt(); }
}

function test($x) {}

test(array(new X));
var_dump($g[2]['args']);
