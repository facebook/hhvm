<?php
<<__EntryPoint>>
function main() {
$post = $GLOBALS['_POST'];
parse_str("a[][]=1&a[][]=3&b[a][b][c]=1&b[a][b][d]=1", &$post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump($_POST['a']);
var_dump($_POST['b']);
}
