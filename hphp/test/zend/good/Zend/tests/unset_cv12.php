<?hh
$x = 1;
function foo() {unset($GLOBALS["x"]);}
call_user_func(fun("foo"));
echo "ok\n";
