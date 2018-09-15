<?php

class A {
  private $foo, $bar;
   function __construct() {
 $this->foo = 1;
 $this->bar = 2;
}
   public function __sleep() {
 $this->foo = 3;
 return array('foo');
}
 }

 <<__EntryPoint>>
function main_747() {
$a = new A();
 var_dump(serialize($a));
}
