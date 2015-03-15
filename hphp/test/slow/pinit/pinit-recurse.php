<?php

function __autoload($cls) {
  echo "__autoloading $cls\n";
  if ($cls === 'Y') {
    var_dump(new X);
  }
}

class X {
  public $p = Y::FOO;
}

function test() {
  for ($i = 0; $i < 3; $i++) {
    try {
      var_dump(new X);
    } catch (Exception $e) {
    }
  }
}

test();
