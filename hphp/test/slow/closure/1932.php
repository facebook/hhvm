<?hh

function f() :mixed{
$someVar = 456;
$closure = function($param) use ($someVar) {
echo $param . ' ' . $someVar . "\n";
}
;
return $closure;
}

<<__EntryPoint>>
function main_1932() :mixed{
$x = f();
$x(2);
call_user_func($x, 2);
}
