<?php
parse_str("a=Hello+World", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

echo $_POST['a']; ?>