<?php

require_once('config.inc');
require_once('lcmess.inc');
	
$dbh = @pg_connect($conn_str);
if (!$dbh) {
	die ("Could not connect to the server");
}

_set_lc_messages();

$res = pg_query($dbh, "CREATE OR REPLACE FUNCTION test_notice() RETURNS boolean AS '
begin
        RAISE NOTICE ''11111'';
        return ''f'';
end;
' LANGUAGE plpgsql;");

$res = pg_query($dbh, 'SET client_min_messages TO NOTICE;');
var_dump($res);
$res = pg_query($dbh, 'SELECT test_notice()');
var_dump($res);
$row = pg_fetch_row($res, 0);
var_dump($row);
pg_free_result($res);
if ($row[0] == 'f')
{
	var_dump(pg_last_notice($dbh));
}

pg_close($dbh);

?>
===DONE===