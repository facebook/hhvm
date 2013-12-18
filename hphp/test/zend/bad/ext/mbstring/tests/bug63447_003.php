<?php
parse_str("a=1&b[][][]=2&c[][][][][][]=7", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

print_r($_POST);
?>