<?php
parse_str("email=foo&password=bar&submit=Log+on", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST);
?>