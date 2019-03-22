<?php
require_once('connect.inc');
if (ZendGoodExtMysqliTestsConnectInc::$skip_on_connect_failure) {
	include_once('connect.inc');
	$link = @my_mysqli_connect(ZendGoodExtMysqliTestsConnectInc::$host, ZendGoodExtMysqliTestsConnectInc::$user, ZendGoodExtMysqliTestsConnectInc::$passwd, ZendGoodExtMysqliTestsConnectInc::$db, ZendGoodExtMysqliTestsConnectInc::$port, ZendGoodExtMysqliTestsConnectInc::$socket);
	if (!is_object($link))
		die(sprintf("skip Can't connect to MySQL Server - [%d] %s", mysqli_connect_errno(), mysqli_connect_error()));
	mysqli_close($link);
}
