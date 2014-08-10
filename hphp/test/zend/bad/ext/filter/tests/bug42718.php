<?php
parse_str("a=1%00", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

echo ini_get('filter.default') . "\n";
echo ini_get('filter.default_flags') . "\n";
var_dump(FILTER_FLAG_STRIP_LOW == 4);
echo addcslashes($_GET['a'],"\0") . "\n";
?>
