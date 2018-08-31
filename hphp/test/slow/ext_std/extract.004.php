<?php

class A {
  public static function b() {
    extract(['this' => 'Hello!']);
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_004() {
A::b();

$a = new A();
$a::b();
$a->b();
}
