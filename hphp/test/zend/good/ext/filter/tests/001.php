<?php
$get = $GLOBALS['_GET'];
parse_str("a=1", &$get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();
echo $_GET['a'];
