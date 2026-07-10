<?hh

function herp($derp) :mixed{}
function foo($bar, $baz) :mixed{}


<<__EntryPoint>>
function main_whitespace_reflectionfunction() :mixed{
var_dump((new ReflectionFunction('herp'))->__toString());
var_dump((new ReflectionFunction('foo'))->__toString());
}
