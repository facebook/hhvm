<?PHP

function a($b, callable $c) {
}


$params = (new ReflectionFunction('a'))->getParameters();
var_dump($params[0]->isCallable());
var_dump($params[1]->isCallable());
