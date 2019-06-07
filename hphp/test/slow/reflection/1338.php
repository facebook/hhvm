<?hh

function foo($a = null) {
}

<<__EntryPoint>>
function main_1338() {
$func = new ReflectionFunction('foo');
$params = $func->getParameters();
var_dump($params[0]->isDefaultValueAvailable());
}
