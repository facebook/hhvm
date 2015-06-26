<?php

function test($cls, ...$args) {
  echo $cls . "\n";
  $a = new $cls(...$args);
  var_dump($a);
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if (($a != $c) && ($c !== null)) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));

  echo "================\n";

  $cls = 'A_' . $cls;
  echo $cls . "\n";
  $a = new $cls(...$args);
  var_dump($a);
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if (($a != $c) && ($c !== null)) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));
}
