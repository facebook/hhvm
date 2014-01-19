<?php

class A {
 public $a = 1;
}
 class B extends A {
   public $m = 10;
  public function test() {
     $b = 'a';
    $this->$b = 'test';
 var_dump($this->$b);
 var_dump($this->a);
    $c = &$this->$b;
 $c = array(1);
 var_dump($this->a);
  }
}
 $obj = new B();
 $obj->test();
