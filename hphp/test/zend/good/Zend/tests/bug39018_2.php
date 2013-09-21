<?php

error_reporting(E_ALL);

$foo = 'test';
$x = @$foo[6];

print @($foo[100] + $foo[130]);

print "\nDone\n";

?>