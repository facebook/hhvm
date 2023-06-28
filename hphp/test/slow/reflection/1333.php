<?hh

class A {
 function foo() :mixed{
}
 }
class B extends A {
 function bar() :mixed{
}
}

<<__EntryPoint>>
function main_1333() :mixed{
var_dump(get_class_methods(new B()));
}
