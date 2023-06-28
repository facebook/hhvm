<?hh

class A {
 const X=1;
 function foo($a = self::X) :mixed{
 var_dump($a);
}
}
 class B extends A {
 const X=2;
 function foo($b = self::X) :mixed{
 var_dump($b);
}
}
 function bar() :mixed{
   $obj = new A;
 $obj->foo();
  $obj = new B;
 $obj->foo();
}

 <<__EntryPoint>>
function main_79() :mixed{
bar();
}
