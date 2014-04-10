<?hh

$f = function () { echo "doo\n"; };
$g = get_class($f);
call_user_func("$g::__invoke");
call_user_func(array($g, "__invoke"));
