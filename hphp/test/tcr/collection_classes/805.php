<?php


$classes = array('Vector','Map','StableMap');
$i = 0;
try {
  foreach ($classes as $cls) {
    $obj = new $cls;
    try {
      $x = $obj->foo;
    } catch (RuntimeException $e) {
      echo $i;
    }
    ++$i;
    try {
      $obj->foo = 123;
    } catch (RuntimeException $e) {
      echo $i;
    }
    ++$i;
  }
} catch (Exception $e) {
  echo "Fail!\n";
}
echo "Done\n";
