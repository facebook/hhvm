<?hh

class A {
}
 class B extends A {
}

<<__EntryPoint>>
function main_1349() :mixed{
$a = new A;
 $b = new B;
var_dump(is_subclass_of($a, 'A'));
var_dump(is_subclass_of($a, 'B'));
var_dump(is_subclass_of($b, 'A'));
var_dump(is_subclass_of($b, 'A', false));
}
