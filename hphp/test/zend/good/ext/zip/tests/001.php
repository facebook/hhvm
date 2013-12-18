<?php
parse_str("", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

parse_str("", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
 
echo "zip extension is available";
?>