<?php

class X {
  function foo() {
    switch ($this) {
    case 'foo': echo 'foo';
 break;
    case 'bar': echo 'bar';
 break;
    default: echo 'def';
    }
  }
  function bar($arg) {
    switch ($this) {
    case $arg: echo 'arg';
 break;
    default: echo 'def';
    }
  }
  function baz($arg) {
    switch ($this) {
    case $arg: echo 'arg';
 break;
    default: echo 'def';
    }
    yield $arg;
  }
}
$x = new X;
$x->foo();
$x->bar(new stdClass);
$x->bar($x);
foreach ($x->baz($x) as $v) {
  var_dump($v);
}
