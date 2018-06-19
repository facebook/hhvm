<?hh

function foo($a = async ($x) ==> { }) {}

$func = new ReflectionFunction('foo');

var_dump($func->getParameters());
