<?php

require_once('config.inc');

$dbh = pg_connect($conn_str);
setlocale(LC_ALL, 'hr_HR.utf-8', 'hr_HR');
echo 3.5.PHP_EOL;
pg_query_params("SELECT $1::numeric", array(3.5));
pg_close($dbh);

echo "Done".PHP_EOL;

?>