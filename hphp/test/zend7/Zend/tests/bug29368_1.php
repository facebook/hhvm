<?php
function throwme($arg)
{
    throw new Exception;
}

class foo {
  function __construct() {
    echo "Inside constructor\n";
    throwme($this);
  }

  function __destruct() {
    echo "Inside destructor\n";
  }
}

try {
  $bar = new foo;
} catch(Exception $exc) {
  echo "Caught exception!\n";
}
?>
