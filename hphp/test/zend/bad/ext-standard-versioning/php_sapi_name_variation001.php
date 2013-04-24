<?php
parse_str("foo=BAR", $_POST);

echo php_sapi_name();
?>