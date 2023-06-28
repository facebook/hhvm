<?hh

interface I1 {
 function ifoo2():mixed;
 function ifoo1():mixed;
 }
interface I2 {
 function ifoo4():mixed;
 function ifoo3():mixed;
 }
class A {
 function foo() :mixed{
}
 function foo2() :mixed{
}
 }
abstract class B extends A implements I1, I2 {
 function bar() :mixed{
}
}
abstract class C extends A implements I2, I1 {
 function bar() :mixed{
}
}
class D extends C {
 function ifoo2() :mixed{
}
 function ifoo1() :mixed{
}
  function ifoo4() :mixed{
}
 function ifoo3() :mixed{
}
 function bar() :mixed{
}
 }

<<__EntryPoint>>
function main_1336() :mixed{
var_dump(get_class_methods('B'));
var_dump(get_class_methods('C'));
}
