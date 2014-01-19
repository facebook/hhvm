<?php

function main() {
  $a = new A;
  $a->e = 1;
  $s = serialize($a);
  var_dump($s);
  var_dump(unserialize($s));
}
class A {
  public $b;
  protected $c;
  private $d;
  public function __sleep() {
    return array('b', 'c', 'd');
  }
}
main();
