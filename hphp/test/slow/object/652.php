<?hh

interface I {
 public function test($a):mixed;
}
 class A {
 public function test($a) :mixed{
 print 'A';
}
}
 class B extends A implements I {
   public function test($a) :mixed{
 print 'B';
}
 }

<<__EntryPoint>>
function main_652() :mixed{
$obj = new A();
 $obj->test(1);
$obj = new B();
 $obj->test(1);
}
