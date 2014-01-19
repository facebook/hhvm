<?php

$classes = array('HH\Vector','Map','StableMap');
$i = 0;
try {
  foreach ($classes as $cls) {
    $obj = new $cls;
    try {
      $x = $obj->foo;
      echo "get: ";
      var_dump($x);
    }
 catch (RuntimeException $e) {
      echo "get throws, i = ";
      var_dump($i);
    }
    try {
      $x = isset($obj->foo);
      echo "isset: ";
      var_dump($x);
    }
 catch (RuntimeException $e) {
      echo "isset throws, i = ";
      var_dump($i);
    }
    try {
      $obj->foo = 123;
    }
 catch (RuntimeException $e) {
      echo "set throws, i = ";
      var_dump($i);
    }
    ++$i;
  }
}
 catch (Exception $e) {
  echo "Fail!\n";
}
echo "Done\n";
