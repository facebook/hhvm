<?hh

class A {
}
 class AA extends A {
 function test() :mixed{
 print 'AA ok';
}
 }
class B {
 function foo(A $obj) :mixed{
 $obj->test();
}
}

<<__EntryPoint>>
function main_724() :mixed{
$obj = new AA();
 $b = new B();
 $b->foo($obj);
}
