<?hh

function herp($derp) :mixed{}
function foo($bar, $baz) :mixed{}


<<__EntryPoint>>
function main_whitespace_reflectionfunction() :mixed{
var_dump((string) (new ReflectionFunction('herp')));
var_dump((string) (new ReflectionFunction('foo')));
}
