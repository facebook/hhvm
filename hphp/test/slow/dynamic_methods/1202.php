<?hh

class A {
 public function test() :mixed{
 print 'in A';
}
 }
 class B extends A {
 public function test() :mixed{
 print 'in B';
}
 }

 <<__EntryPoint>>
function main_1202() :mixed{
$obj = new B();
 call_user_func_array(vec[$obj, 'A::test'], vec[]);
}
