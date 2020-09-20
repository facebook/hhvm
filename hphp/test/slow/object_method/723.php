<?hh

class A {
 function test() {
 print 'A';
}
 function foo() {
 $this->test();
}
}
 class B extends A {
 function test() {
 print 'B';
}
}

 <<__EntryPoint>>
function main_723() {
$obj = new A();
 $obj = new B();
 $obj->foo();
}
