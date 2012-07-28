<?php

print "Test begin\n";

class A {}
interface I {
  public function a($x, array $a = array());
  public function b(array $a = null, $x);
  public function c($x, A $a1, A $a2=null, A $a3, $y);
  public function d(array $a = null, $x=0, $y);
}
class B implements I {
  public function a($x, array $a = array()) {}
  public function b(array $a = null, $x) {}
  public function c($x, A $a1, A $a2=null, A $a3, $y) {}
  public function d(array $a = null, $x, $y=0) {}
}
class C implements I {
  public function a($x=0, array $a = null) {}
  public function b(array $a = array(), $x=0) {}
  public function c($x, A $a1=null, A $a2, A $a3=null, $y, $z=0) {}
  public function d(array $a = null, $x, $y) {}
}

print "Test end\n";
