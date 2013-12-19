<?php
$_COOKIE = http_parse_cookie("PHPSESSID=rfc1867-tests");

parse_str("PHPSESSID=rfc1867-tests-get", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

session_start();
var_dump(session_id());
var_dump($_FILES);
var_dump($_SESSION["upload_progress_" . basename(__FILE__)]);
session_destroy();
?>