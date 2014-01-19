<?php

class A {
  private $b = 'b';
  protected $c = 'c';
  public $d = 'd';
}
class E {
  static private $f = 'f';
  static protected $g = 'g';
  static public $h = 'h';
}

function getProps($class, $obj) {
  foreach ((new ReflectionClass($class))->getProperties() as $key => $prop) {
    $values = array();

    $key = $prop->getName();

    $prop->setAccessible(true);
    $values[] = $prop->getValue($obj);

    $prop->setValue($obj, 'newval');
    $values[] = $prop->getValue($obj);

    $ret[$key] = $values;
  }
  return $ret;
}

$ret = array_merge(getProps('A', new A), getProps('E', 'E'));
ksort($ret);
var_dump($ret);
