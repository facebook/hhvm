<?php

function foo(&$ref) { $ref = 24; }

$a = [];
call_user_func('foo', $a[0][0]);
var_dump($a);

?>
