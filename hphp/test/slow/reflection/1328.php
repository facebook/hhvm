<?hh

class A {
 public function foo() :mixed{
}
}

<<__EntryPoint>>
function main_1328() :mixed{
$x = new ReflectionMethod('A::foo');
var_dump($x->name, $x->class);
}
