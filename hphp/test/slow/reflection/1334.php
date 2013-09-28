<?php

class X {
  public function a() {
 var_dump(get_class_methods($this));
 }
  protected function b() {
}
  private function c() {
}
  final function d() {
}
  public function e() {
}
}
class Y {
  public function a() {
 var_dump(get_class_methods($this));
 }
}
$x = new X;
$x->a();
$y = new Y;
$y->a();
var_dump(get_class_methods($x));
var_dump(get_class_methods($y));
