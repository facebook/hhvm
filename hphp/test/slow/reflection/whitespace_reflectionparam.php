<?hh

function foo($bar) :mixed{}


<<__EntryPoint>>
function main_whitespace_reflectionparam() :mixed{
$rf = new ReflectionFunction('foo');
var_dump((string) $rf->getParameters()[0]);
}
