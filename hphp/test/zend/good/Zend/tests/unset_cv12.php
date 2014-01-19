<?php
$x = 1;
function foo() {unset($GLOBALS["x"]);}
call_user_func("foo");
echo "ok\n";
?>