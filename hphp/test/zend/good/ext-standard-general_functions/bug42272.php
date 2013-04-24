<?php
$foo = var_export("\0", true );
echo $foo, "\n";
var_export("a\0b");
?>