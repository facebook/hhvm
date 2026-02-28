<?hh

class A {
}
 class B extends A {
}

<<__EntryPoint>>
function main_1346() :mixed{
$a = new A;
 $b = new B;
var_dump(is_a($a, 'A'));
var_dump(is_a($a, 'B'));
var_dump(is_a($b, 'A'));
var_dump(is_a($b, 'A', true));
}
