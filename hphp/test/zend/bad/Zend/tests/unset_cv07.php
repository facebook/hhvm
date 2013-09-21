<?php
parse_str("x=2", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

$_x = "1\n";
echo $_x;
import_request_variables("g","_");
echo $_x;
echo "\nok\n";
?>