<?php

// Test destruction order for SetG.
class d {
  public function __destruct() {
    $GLOBALS['x'] = "destructor";
  }
}

function foo() {
  $GLOBALS['x'] = new d();
  echo "Foo: ";
  echo ($GLOBALS['x'] = "main");
  echo "\n";
  var_dump($GLOBALS['x']);
}

foo();
