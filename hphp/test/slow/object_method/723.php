<?hh

class A {
 function test() :mixed{
 print 'A';
}
 function foo() :mixed{
 $this->test();
}
}
 class B extends A {
 function test() :mixed{
 print 'B';
}
}

 <<__EntryPoint>>
function main_723() :mixed{
$obj = new A();
 $obj = new B();
 $obj->foo();
}
