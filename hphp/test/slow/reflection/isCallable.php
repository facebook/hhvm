<?hh

function a($b, callable $c) {
}



<<__EntryPoint>>
function main_is_callable() {
$params = (new ReflectionFunction('a'))->getParameters();
var_dump($params[0]->isCallable());
var_dump($params[1]->isCallable());
}
