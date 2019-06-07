<?hh

function foo($bar) {}


<<__EntryPoint>>
function main_whitespace_reflectionparam() {
$rf = new ReflectionFunction('foo');
var_dump((string) $rf->getParameters()[0]);
}
