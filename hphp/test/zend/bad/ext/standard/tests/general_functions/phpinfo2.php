<?php
parse_str("dummy=x", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump(phpinfo());

echo "--\n";
var_dump(phpinfo(array()));

echo "--\n";
var_dump(phpinfo(0));

echo "--\n";
var_dump(phpinfo(INFO_LICENSE));

?>