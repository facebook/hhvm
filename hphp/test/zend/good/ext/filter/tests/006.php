<?php
parse_str("foo=<b>abc</b>", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
 
echo filter_input(INPUT_POST, 'foo', FILTER_SANITIZE_STRIPPED);
?>