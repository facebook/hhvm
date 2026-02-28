<?hh

interface A {
 function foo():mixed;
 }
abstract class B implements A {
 function bar() :mixed{
}
}

<<__EntryPoint>>
function main_1335() :mixed{
var_dump(get_class_methods('B'));
}
