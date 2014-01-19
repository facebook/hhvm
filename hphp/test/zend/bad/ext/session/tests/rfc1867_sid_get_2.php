<?php
$_COOKIE = http_parse_cookie("PHPSESSID=rfc1867-tests-cookie");
_filter_snapshot_globals();

parse_str("PHPSESSID=rfc1867-tests", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

session_start();
var_dump(session_id());
var_dump(basename(__FILE__) == $_POST[ini_get("session.upload_progress.name")]);
var_dump($_FILES);
var_dump($_SESSION["upload_progress_" . basename(__FILE__)]);
session_destroy();
?>