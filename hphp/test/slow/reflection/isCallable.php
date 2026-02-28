<?hh

function a($b, callable $c) :mixed{
}



<<__EntryPoint>>
function main_is_callable() :mixed{
$params = (new ReflectionFunction('a'))->getParameters();
var_dump($params[0]->isCallable());
var_dump($params[1]->isCallable());
}
