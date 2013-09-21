<?php

include('config.inc');

$db = pg_connect($conn_str);

echo "import LO from int\n";
pg_exec($db, 'begin');
$oid = pg_lo_import($db, __FILE__, 21003);
if (!$oid) echo ("pg_lo_import() error\n");
if ($oid != 21003) echo ("pg_lo_import() wrong id\n");
pg_lo_unlink ($db, $oid);
pg_exec($db, 'commit');

echo "import LO from string\n";
pg_exec($db, 'begin');
$oid = pg_lo_import($db, __FILE__, "21004");
if (!$oid) echo ("pg_lo_import() error\n");
if ($oid != 21004) echo ("pg_lo_import() wrong id\n");
pg_lo_unlink ($db, $oid);
pg_exec($db, 'commit');

echo "import LO using default connection\n";
pg_exec('begin');
$oid = pg_lo_import($db, __FILE__, 21005);
if (!$oid) echo ("pg_lo_import() error\n");
if ($oid != 21005) echo ("pg_lo_import() wrong id\n");
pg_lo_unlink ($oid);
pg_exec('commit');


echo "OK";
?>