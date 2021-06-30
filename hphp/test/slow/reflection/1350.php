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
var_dump(is_subclass_of('A', 'A', true));
var_dump(is_subclass_of('A', 'A', false));
var_dump(is_subclass_of('B', 'A', true));
var_dump(is_subclass_of('A', 'B', true));
var_dump(is_subclass_of('C', 'A', true));
}
