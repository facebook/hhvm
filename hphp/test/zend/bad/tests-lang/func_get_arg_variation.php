<?php

function foo($a)
{
   $a=5;
   echo func_get_arg();
   echo func_get_arg(2,2);
   echo func_get_arg("hello");
   echo func_get_arg(-1);
   echo func_get_arg(2);
}
foo(2);
echo "\n";
?>