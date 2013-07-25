<?php
parse_str("foo=BAR", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

echo php_sapi_name();
?>