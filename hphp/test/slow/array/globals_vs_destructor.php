<?php

class C {
  public function __destruct() {
    echo "C::__destruct\n";
    global $obj;
    var_dump($obj);
    $obj = 23;
    echo "C::__destruct done\n";
  }
}

function test() {
  $a = $GLOBALS;
  $a['obj'] = new C;
  $a['obj'] = 42;
  var_dump($a['obj']);
}

test();
