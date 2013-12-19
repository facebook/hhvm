<?php
parse_str("a=1&b=ZYX&c[][][][][][][][][][][][][][][][][][][][][][]=123&d=123&e[][]][]=3", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST, $php_errormsg);
?>