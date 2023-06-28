<?hh

function foo($a = null) :mixed{
}

<<__EntryPoint>>
function main_1338() :mixed{
$func = new ReflectionFunction('foo');
$params = $func->getParameters();
var_dump($params[0]->isDefaultValueAvailable());
}
