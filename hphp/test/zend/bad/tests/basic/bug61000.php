<?php
parse_str("a[a][]=foo&a[a][b][c]=bar", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

parse_str("1[a][]=foo&1[a][b][c]=bar", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

print_r($_GET);
print_r($_POST);