<?php
<<__EntryPoint>>
function main() {
$post = $GLOBALS['_POST'];
parse_str("foo=<b>abc</b>", &$post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

echo filter_input(INPUT_POST, 'foo', FILTER_SANITIZE_STRIPPED);
}
