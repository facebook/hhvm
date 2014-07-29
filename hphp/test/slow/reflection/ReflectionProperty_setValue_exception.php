<?php

class cls {
  private $priv = 42;
  private static $s_priv = 21;
}

$prop = new ReflectionProperty('cls', 'priv');
$s_prop = new ReflectionProperty('cls', 's_priv');

// Non-static
try {
  $obj = new cls();
  $prop->setValue($obj, 1);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}

// Static
try {
  $s_prop->setValue(1);
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
