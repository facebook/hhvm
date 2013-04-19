<?php

class X {
  function __construct() {
  }
}

function handler($kind, $name) {
  if ($kind == 'exit' && $name == 'X::__construct') throw new Exception;
}

function test() {
  fb_setprofile('handler');
  try {
    new X;
  } catch (Exception $e) {
    echo "ok\n";
  }
}

test();
