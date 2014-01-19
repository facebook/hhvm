<?php

class Foo {
  public $a;
  static public $b;
  static public $c;
}
$foo = new Foo;
$foo->a = function ($x) {
 echo '!' . $x;
 }
;
($foo->a)("foo\n");
Foo::$b = function ($x) {
 echo '?' . $x;
 }
;
(Foo::$b)("bar\n");
Foo::$c[0] = function ($x) {
 echo '.' . $x;
 }
;
(Foo::$c[0])("baz\n");
