<?php

$a = function ($v) {
 return $v > 2;
 }
;
 echo $a(4)."
";
 echo call_user_func_array($a, array(4));
