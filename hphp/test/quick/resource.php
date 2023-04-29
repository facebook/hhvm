<?hh

class Foo{}

function f(resource $r) {
}
<<__EntryPoint>> function main(): void {
$r = HH\stdin();
var_dump($r);
var_dump($r is resource);
f($r);

$foo = new Foo();
var_dump($foo);
var_dump($foo is resource);
f($foo);
}
