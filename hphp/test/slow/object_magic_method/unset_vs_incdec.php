<?php

class C {
  public $foo = 1;
  public $bar = 2;
  function &__get($name) {
    if ($name == 'foo') {
      return $this->bar;
    }
  }
}

$c = new C();
unset($c->foo);
var_dump($c->foo++);
var_dump($c);
