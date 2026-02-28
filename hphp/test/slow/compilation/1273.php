<?hh

class A {
 function test() :mixed{
}
}
 class B {
 public $b;
}
 class C {
 function test() :mixed{
}
}

 <<__EntryPoint>>
function main_1273() :mixed{
$a = 'test';
 $a = new B();
 $a->b = new A();
 $a->b->test();
}
