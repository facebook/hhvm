<?hh

class A {
 function test() {
}
}
 class B {
 public $b;
}
 class C {
 function test() {
}
}

 <<__EntryPoint>>
function main_1273() {
$a = 'test';
 $a = new B();
 $a->b = new A();
 $a->b->test();
}
