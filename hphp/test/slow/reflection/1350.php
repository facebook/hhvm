<?hh

class A {
}
class B extends A {
}
class C extends B {
}

<<__EntryPoint>>
function main_1350() {
$a = new A;
 $b = new B;
var_dump(is_subclass_of('a', 'A', true));
var_dump(is_subclass_of('a', 'A', false));
var_dump(is_subclass_of('b', 'A', true));
var_dump(is_subclass_of('a', 'B', true));
var_dump(is_subclass_of('c', 'A', true));
}
