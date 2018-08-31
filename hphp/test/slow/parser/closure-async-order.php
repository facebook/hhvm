<?hh

function foo($a = async ($x) ==> { }) {}


<<__EntryPoint>>
function main_closure_async_order() {
$func = new ReflectionFunction('foo');

var_dump($func->getParameters());
}
