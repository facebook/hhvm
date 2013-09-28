<?php
class Foo {
  var $foo = "test";
}

$f = new Foo();
$f->bar =& $f->foo;
var_dump($f->foo);
var_dump($f->bar);
?>