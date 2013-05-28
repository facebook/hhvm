<?php

class A {
  public $foo = 0;
  public $fooref = 1;
  private $foopriv = 2;
  function __clone() {
    echo "clone
";
  }
}
$a1 = new A;
$p = 8;
$q = 9;
$a1->foo = 'foo';
$a1->fooref = &$p;
$a1->dyn = 'dyn';
$a1->dynref = &$q;
var_dump($a1);
$a2 = clone $a1;
var_dump($a1);
var_dump($a2);
$a2->foo = 'a2foo';
$a2->fooref = 'a2fooref';
$a2->dyn = 'a2dyn';
$a2->dynref = 'a2dynref';
$a2->dynref2 = 'dynref2';
var_dump($a1);
var_dump($a2);
var_dump($p);
var_dump($q);
