<?php
parse_str("a=Hello+World&b=Hello+Again+World", $_POST);
 
error_reporting(0);
echo "{$_POST['a']} {$_POST['b']}" ?>