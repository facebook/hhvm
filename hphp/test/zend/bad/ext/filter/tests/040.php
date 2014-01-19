<?php
parse_str("a=1&b=2&c=0", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

parse_str("ap[]=1&bp=test&cp=", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();


var_dump(filter_has_var());
var_dump(filter_has_var(INPUT_GET,""));
var_dump(filter_has_var(INPUT_GET,array()));
var_dump(filter_has_var(INPUT_POST, "ap"));
var_dump(filter_has_var(INPUT_POST, "cp"));
var_dump(filter_has_var(INPUT_GET, "a"));
var_dump(filter_has_var(INPUT_GET, "c"));
var_dump(filter_has_var(INPUT_GET, "abc"));
var_dump(filter_has_var(INPUT_GET, "cc"));
var_dump(filter_has_var(-1, "cc"));
var_dump(filter_has_var(0, "cc"));
var_dump(filter_has_var("", "cc"));

echo "Done\n";
?>