<?php
parse_str("a=qwe&abc=<a>href</a>", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

parse_str("b=qwe&bbc=<a>href</a>", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();


var_dump(filter_has_var(INPUT_GET, "a"));
var_dump(filter_has_var(INPUT_GET, "abc"));
var_dump(filter_has_var(INPUT_GET, "nonex"));
var_dump(filter_has_var(INPUT_GET, " "));
var_dump(filter_has_var(INPUT_GET, ""));
var_dump(filter_has_var(INPUT_GET, array()));

var_dump(filter_has_var(INPUT_POST, "b"));
var_dump(filter_has_var(INPUT_POST, "bbc"));
var_dump(filter_has_var(INPUT_POST, "nonex"));
var_dump(filter_has_var(INPUT_POST, " "));
var_dump(filter_has_var(INPUT_POST, ""));
var_dump(filter_has_var(INPUT_POST, array()));

var_dump(filter_has_var(-1, ""));
var_dump(filter_has_var("", ""));
var_dump(filter_has_var(array(), array()));
var_dump(filter_has_var(array(), ""));
var_dump(filter_has_var("", array()));

echo "Done\n";
?>