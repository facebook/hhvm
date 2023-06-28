<?hh

class A {
  function a() :mixed{
 echo "A
";
 }
  function __construct() {
 echo "cons
";
 }
}
 function test() :mixed{
 $obj = new A();
 $obj->a();
 }

 <<__EntryPoint>>
function main_1720() :mixed{
test();
}
