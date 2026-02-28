<?hh

abstract class A {
  abstract public function __invoke($x):mixed;
}
interface IfaceInvoke {
  public function __invoke($x):mixed;
}
class Test1 extends A {
  public function __invoke($x) :mixed{
    var_dump(__CLASS__);
    var_dump($x);
  }
}
class Test2 implements IfaceInvoke {
  public function __invoke($x) :mixed{
    var_dump(__CLASS__);
    var_dump($x);
  }
}
function f1($x, $y)             :mixed{
 $x($y);
 $x->__invoke($y);
 }
function f2(A $x, $y)           :mixed{
 $x($y);
 $x->__invoke($y);
 }
function f3(IfaceInvoke $x, $y) :mixed{
 $x($y);
 $x->__invoke($y);
 }

<<__EntryPoint>>
function main_773() :mixed{
$t1 = new Test1;
$t2 = new Test2;
f1($t1, 1);
f1($t2, 2);
f2($t1, 1);
f3($t2, 2);
}
