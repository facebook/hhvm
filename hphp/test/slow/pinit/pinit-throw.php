<?php

function __autoload($cls) {
  echo "__autoloading $cls\n";
  if ($cls === 'Y') {
    throw new Exception("x");
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
      echo "Caught exception constructing an X: ".$e->getMessage()."\n";
    }
  }
}

test();
