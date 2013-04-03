<?php
parse_str("a=Hello+World&b=Hello+Again+World&c=1", $_POST);
 
error_reporting(0);
echo "{$_POST['a']} {$_POST['b']} {$_POST['c']}"?>