<?hh

class A {
 function foo() {
}
 }
class B extends A {
 function bar() {
}
}

<<__EntryPoint>>
function main_1333() {
var_dump(get_class_methods(new B()));
}
