<?hh

function test1($args) :mixed{}
function test2(...$args) :mixed{}
function test3($arg, ...$args) :mixed{}
<<__EntryPoint>> function main(): void {
var_dump((new ReflectionFunction('test1'))->isVariadic());
var_dump((new ReflectionFunction('test2'))->isVariadic());
var_dump((new ReflectionFunction('test3'))->isVariadic());
}
