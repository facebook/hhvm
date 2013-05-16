<?php
require_once('config.inc');
	
$dbh = @pg_connect($conn_str);
if (!$dbh) {
	die ("Could not connect to the server");
}
pg_close($dbh);

require_once(dirname(__FILE__).'/../../dba/tests/test.inc');
require_once(dirname(__FILE__).'/../../dba/tests/dba_handler.inc');

?>