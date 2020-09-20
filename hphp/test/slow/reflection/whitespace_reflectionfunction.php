<?hh

function herp($derp) {}
function foo($bar, $baz) {}


<<__EntryPoint>>
function main_whitespace_reflectionfunction() {
var_dump((string) (new ReflectionFunction('herp')));
var_dump((string) (new ReflectionFunction('foo')));
}
