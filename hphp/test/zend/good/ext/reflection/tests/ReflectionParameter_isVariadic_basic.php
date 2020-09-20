<?hh

function test1($args) {}
function test2(...$args) {}
function test3($arg, ...$args) {}
<<__EntryPoint>> function main(): void {
$r1 = new ReflectionFunction('test1');
$r2 = new ReflectionFunction('test2');
$r3 = new ReflectionFunction('test3');

var_dump($r1->getParameters()[0]->isVariadic());
var_dump($r2->getParameters()[0]->isVariadic());
var_dump($r3->getParameters()[0]->isVariadic());
var_dump($r3->getParameters()[1]->isVariadic());
}
