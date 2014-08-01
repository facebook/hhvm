<?php
include 'config.inc';
include 'lcmess.inc';

$db = pg_connect($conn_str);

_set_lc_messages();

$res = pg_query($db, 'SET client_min_messages TO NOTICE;'); 
var_dump($res);

pg_query($db, "BEGIN;");
pg_query($db, "BEGIN;");

$msg = pg_last_notice($db);
if ($msg === FALSE) {
	echo "Cannot find notice message in hash\n";
	var_dump($msg);
}
echo $msg."\n";
echo "pg_last_notice() is Ok\n";

?>