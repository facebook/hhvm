<?php

$myfunc = function() {
  echo "hello, world!\n";
}
;
$myfunc();
call_user_func($myfunc);
call_user_func_array($myfunc, array());
$isc = is_callable($myfunc, false, &$p);
echo "is_callable(\$myfunc) = $isc\n";
var_dump($p);
