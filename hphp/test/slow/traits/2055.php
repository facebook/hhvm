<?php

trait T1 {
  public function inc($who) {
    static $x=0;
    $x++;
    echo $who . " (" . __class__ . "): " . $x . "\n";
  }
}
class B {
 use T1;
 }
class C {
 use T1;
 }
class D extends C {
}
$c1 = new C;
$c2 = new C;
$d1 = new D;
$b1 = new B;
$c1->inc("c1");
$c2->inc("c2");
$d1->inc("d1");
$b1->inc("b1");
$b1->inc("b1");
$c2->inc("c2");
$d1->inc("d1");
$c1->inc("c1");
