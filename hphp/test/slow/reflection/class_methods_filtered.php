<?php
abstract class A {
  private function f() {}
  abstract protected function g();
  abstract public function h();
}

interface I {
    public function i();
    public function j();
    static function s();
}

abstract class B extends A implements I {
  protected function g(){}
  public function h(){}
  public function j() {}
}

$ref = new ReflectionClass("B");
var_dump($ref->getMethods(ReflectionMethod::IS_ABSTRACT));
var_dump($ref->getMethods(ReflectionMethod::IS_STATIC));
