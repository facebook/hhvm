<?hh

class A {
 public function foo() {
}
}

<<__EntryPoint>>
function main_1328() {
$x = new ReflectionMethod('A::foo');
var_dump($x->name, $x->class);
}
