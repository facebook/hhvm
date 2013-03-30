<?php

$b = function() { return func_get_args(); };
$a = 'b';
var_dump($$a(1));
var_dump($$a->__invoke(2));

?>