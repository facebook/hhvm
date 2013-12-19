<?php
parse_str("a=ABC&y=XYZ&c[]=1&c[]=2&c[a]=3", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST, $HTTP_RAW_POST_DATA);
?>