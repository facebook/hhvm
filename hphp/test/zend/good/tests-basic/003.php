<?php
parse_str("b=Hello+Again+World&c=Hi+Mom", $_GET);

parse_str("a=Hello+World", $_POST);
 
error_reporting(0);
echo "post-a=({$_POST['a']}) get-b=({$_GET['b']}) get-c=({$_GET['c']})"?>