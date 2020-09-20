<?hh

class A {
}
 class AA extends A {
 function test() {
 print 'AA ok';
}
 }
class B {
 function foo(A $obj) {
 $obj->test();
}
}

<<__EntryPoint>>
function main_724() {
$obj = new AA();
 $b = new B();
 $b->foo($obj);
}
