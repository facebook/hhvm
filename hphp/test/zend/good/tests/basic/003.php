<?php
parse_str("b=Hello+Again+World&c=Hi+Mom", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

parse_str("a=Hello+World", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
 
error_reporting(0);
echo "post-a=({$_POST['a']}) get-b=({$_GET['b']}) get-c=({$_GET['c']})"?>