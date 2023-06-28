<?hh

function foo($a = async ($x) ==> { }) :mixed{}


<<__EntryPoint>>
function main_closure_async_order() :mixed{
$func = new ReflectionFunction('foo');

var_dump($func->getParameters());
}
