<?php
parse_str("dummy=1", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

header("Vary: Cookie");
ob_start("ob_gzhandler");

// run-tests cannot test for a multiple Vary header
ob_flush();
print_r(headers_list());

?>
Done