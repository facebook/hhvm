<?hh

class A {
}
class B extends A {
}
class C extends B {
}

<<__EntryPoint>>
function main_1347() {
$a = new A;
 $b = new B;
var_dump(is_a('a', 'A', true));
var_dump(is_a('a', 'A', false));
var_dump(is_a('b', 'A', true));
var_dump(is_a('a', 'B', true));
var_dump(is_a('c', 'A', true));
}
