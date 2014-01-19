<?php
require_once __DIR__."/../utils/server.inc";

$host = hostname();
$port = standalone_port();
$dbname = dbname();
$username = "";
$password = "";

$m = new mongo("$host:$port");
$db = $m->selectDB(dbname());
$result = $db->authenticate($username, $password);
echo (int) $result['ok'] . "\n";

$result = $db->authenticate($username, $password.'wrongPass');
echo (int) $result['ok'] . "\n";
?>