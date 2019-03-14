<?php

<<__EntryPoint>>
function main() {
$get = $GLOBALS['_GET'];
parse_str("ar[elm1]=1234&ar[elm2]=0660&a=0234", &$get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

$post = $GLOBALS['_POST'];
parse_str("d=379", &$post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

$ret = filter_input(INPUT_GET, 'a', FILTER_VALIDATE_INT);
var_dump($ret);

$ret = filter_input(INPUT_GET, 'a', FILTER_VALIDATE_INT, array('flags'=>FILTER_FLAG_ALLOW_OCTAL));
var_dump($ret);

$ret = filter_input(INPUT_GET, 'ar', FILTER_VALIDATE_INT, array('flags'=>FILTER_REQUIRE_ARRAY));
var_dump($ret);

$ret = filter_input(INPUT_GET, 'ar', FILTER_VALIDATE_INT, array('flags'=>FILTER_FLAG_ALLOW_OCTAL|FILTER_REQUIRE_ARRAY));
var_dump($ret);
}
