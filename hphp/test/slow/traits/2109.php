<?php

trait T {
  public function init() {
    parent::init();
  }
}
class A {
  public function init() {
    echo 'A::init';
  }
}
class B extends A {
  use T;
}
class C extends B {
  public function init() {
    parent::init();
  }
}
$obj = new C;
$obj->init();
