<?php


$ctypes = array('Vector', 'Map', 'StableMap');
foreach ($ctypes as $ctype) {
  echo "=== $ctype ===\n";
  $c = new $ctype();
  try {
    $c[0];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $c[PHP_INT_MAX];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $c[~PHP_INT_MAX];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  if ($ctype === 'Vector') {
    continue;
  }
  try {
    $c['abc'];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $c['abcdefghijklmnopqrst'];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $c['abcdefghijklmnopqrstu'];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $c["abcdefghij\000klmnopqrst"];
  } catch (Exception $e) {
    $str = $e->getMessage();
    $i = 0;
    for (;;) {
      echo(ord($str[$i]));
      ++$i;
      if ($i >= strlen($str)) {
        break;
      }
      if (($i % 8) === 0) {
        echo "\n";
      } else {
        echo ' ';
      }
    }
    echo "\n";
  }
}
