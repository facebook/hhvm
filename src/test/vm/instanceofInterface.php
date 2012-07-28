<?php


interface Interf1 { }
interface Interf2 { }
interface Interf3 { }

abstract class AC1 implements Interf2, Interf1 { }
abstract class AC2 extends AC1 { }
abstract class AC3 extends AC2 implements Interf3 { }
abstract class AC4 extends AC3 { }

class C1 extends AC4 { }

function array_some(array $array) {
  foreach ($array as $value) {
    if ($value) {
      echo "Empty: ";
      echo empty($value);
      echo "\nBool: ";
      echo $value ? "true" : "false";
      echo "\n";
    }
  }
  return "Done";
}

$a = new C1;

var_dump( array_some(array_map(function($v) { return $v instanceof Interf2; },
                     array($a))) );

