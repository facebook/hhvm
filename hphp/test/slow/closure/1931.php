<?hh


<<__EntryPoint>>
function main_1931() :mixed{
$myfunc = function() {
  echo "hello, world!\n";
}
;
$myfunc();
call_user_func($myfunc);
call_user_func_array($myfunc, vec[]);
$p = null;
$isc = is_callable_with_name($myfunc, false, inout $p);
$isc__str = (string)($isc);
echo "is_callable(\$myfunc) = $isc__str\n";
var_dump($p);
}
