<?php

$a = str_repeat("a", 10 * 1024 * 1024);

eval("class $a {}");

# call_user_func($a); // Warning
# $a->$a();           // Fatal error

if ($a instanceof $a); // Segmentation fault
new $a;                // Segmentation fault
echo "ok\n";