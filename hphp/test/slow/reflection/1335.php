<?hh

interface A {
 function foo();
 }
abstract class B implements A {
 function bar() {
}
}

<<__EntryPoint>>
function main_1335() {
var_dump(get_class_methods('B'));
}
