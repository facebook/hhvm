<?php
$post = $_POST;
parse_str("a=Hello+World", &$post);
$_POST = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

echo $_POST['a'];
