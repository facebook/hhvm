<?php
$post = $_POST;
parse_str("a[]=1&a[0]=5", &$post);
$_POST = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump($_POST['a']);
?>
