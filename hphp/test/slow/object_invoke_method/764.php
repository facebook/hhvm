<?php

// standard execution
class C1 {
  public function __invoke($a0, $a1) {
    var_dump('C1');
    var_dump($a0, $a1);
  }
}
class D1 extends C1 {
}
class E1 extends D1 {
  public function __invoke($a0, $a1) {
    var_dump('D2');
    var_dump($a0, $a1);
  }
}
class F1 {
  public function __invoke($a0) {
    return $a0 > 10;
  }
}
$c = new C1;
$d = new D1;
$e = new E1;
$c(0, 1);
$d(0, 1);
$e(0, 1);
call_user_func($c, 0, 1);
call_user_func($d, 0, 1);
call_user_func($e, 0, 1);
call_user_func_array($c, array(0, 1));
call_user_func_array($d, array(0, 1));
call_user_func_array($e, array(0, 1));
$c->__invoke(0, 1);
$d->__invoke(0, 1);
$e->__invoke(0, 1);
C1::__invoke(0, 1);
D1::__invoke(0, 1);
E1::__invoke(0, 1);
function mk($n) {
 return $n . '::__invoke';
 }
call_user_func(mk('C1'), 0, 1);
call_user_func(mk('D1'), 0, 1);
call_user_func(mk('E1'), 0, 1);
var_dump(array_filter(array(0, 1, 11, 13), new F1));
