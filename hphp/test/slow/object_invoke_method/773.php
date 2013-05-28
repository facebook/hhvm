<?php

abstract class A {
  abstract public function __invoke($x);
}
interface IfaceInvoke {
  public function __invoke($x);
}
class Test1 extends A {
  public function __invoke($x) {
    var_dump(__CLASS__);
    var_dump($x);
  }
}
class Test2 implements IfaceInvoke {
  public function __invoke($x) {
    var_dump(__CLASS__);
    var_dump($x);
  }
}
function f1($x, $y)             {
 $x($y);
 $x->__invoke($y);
 }
function f2(A $x, $y)           {
 $x($y);
 $x->__invoke($y);
 }
function f3(IfaceInvoke $x, $y) {
 $x($y);
 $x->__invoke($y);
 }
$t1 = new Test1;
$t2 = new Test2;
f1($t1, 1);
f1($t2, 2);
f2($t1, 1);
f3($t2, 2);
