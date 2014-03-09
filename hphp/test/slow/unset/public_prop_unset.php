<?php

class Foo {
  private $private;
  protected $protected;
  public $public;
  public function __unset($prop) { echo "__unset($prop)\n"; }
}

$obj = new Foo();

unset($obj->private);
unset($obj->private);
unset($obj->protected);
unset($obj->protected);
unset($obj->public);
unset($obj->public);
