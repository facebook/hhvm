<?php

class A {
  public static function b() {
    extract(['this' => 'Hello!']);
    var_dump($this);
  }
}

A::b();

$a = new A();
$a::b();
$a->b();
