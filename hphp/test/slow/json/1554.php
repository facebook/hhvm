<?php

class A {
  public $a = 'foo';
  protected $b = 'bar';
  private $c = 'blah';
  public function aaaa() {
    var_dump(json_encode($this));
  }
}
$obj = new A();
$obj->aaaa();
