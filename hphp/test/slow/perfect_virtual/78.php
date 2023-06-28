<?hh

class A {
 function foo(...$args) :mixed{
 var_dump(__CLASS__, $args);
}
}
 class B extends A {
 function foo(...$args) :mixed{
 var_dump(__CLASS__, $args);
}
}
 function bar() :mixed{
   $obj = new A;
 $obj->foo(123);
  $obj = new B;
 $obj->foo(123, 456);
}

 <<__EntryPoint>>
function main_78() :mixed{
bar();
}
