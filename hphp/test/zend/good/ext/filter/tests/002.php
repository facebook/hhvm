<?php

<<__EntryPoint>>
function main() {
$get = $GLOBALS['_GET'];
parse_str("a=1&b=&c=3", &$get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();
echo $_GET['a'];
echo $_GET['b'];
echo $_GET['c'];
}
