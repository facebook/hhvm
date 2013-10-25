<?php
require __DIR__."/../utils/server.inc";

$m = mongo_standalone("admin");
$dbs = $m->listDBs();
var_dump($dbs['ok']);
var_dump(isset($dbs['totalSize']));
var_dump(isset($dbs['databases']) && is_array($dbs['databases']));
?>