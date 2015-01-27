<?php

class A extends __PHP_Incomplete_Class {
}

function main() {
  $a = unserialize('O:14:"BogusTestClass":0:{}');
  var_dump(is_object($a));
  var_dump($a instanceof __PHP_Incomplete_Class);

  $a = new __PHP_Incomplete_Class();
  var_dump(is_object($a));


  $a = new A();
  var_dump(is_object($a));

  $a = 100500;
  var_dump(is_object($a));

  $b = "ololo";
  var_dump(is_object($b));

  // The type not known at compile-time
  $c = new stdClass();
  var_dump(is_object($c));

  // The type known at compile-time
  var_dump(is_object(new stdclass));
}

main();
