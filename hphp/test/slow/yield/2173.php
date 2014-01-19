<?php

class A {
  public static function sgen() {
    $class = get_called_class();
    yield $class;
  }
  public static function sfoo() {
    return self::gen();
  }
  public function gen() {
    $class = get_called_class();
    yield $class;
  }
  public function foo() {
    return self::gen();
  }
}
class B extends A {
}
function t($x) {
 foreach ($x as $v) {
 var_dump($v);
 }
 }
t(B::sgen());
t(B::sfoo());
t(A::sgen());
t(A::sfoo());
t(B::gen());
t(B::foo());
t(A::gen());
t(A::foo());
$b = new B;
t($b->gen());
t($b->foo());
$a = new A;
t($a->gen());
t($a->foo());
