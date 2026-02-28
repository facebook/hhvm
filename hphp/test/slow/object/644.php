<?hh

class A {
 public $a = 0;
}
 class B extends A {
}

<<__EntryPoint>>
function main_644() :mixed{
$obj1 = new A();
 $obj2 = new A();
 $obj2->a++;
 $obj3 = new B();
 $obj3->a = 10;
var_dump($obj1->a);
var_dump($obj1);
var_dump($obj2->a);
var_dump($obj2);
var_dump($obj3);
var_dump($obj1 is A);
var_dump($obj3 is A);
var_dump($obj1 is B);
var_dump($obj3 is B);
}
