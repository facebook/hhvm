<?php
require __DIR__."/../utils/server.inc";

$m = mongo_standalone("admin");
$db = $m->selectDb("admin");

$status = $db->command(array('serverStatus' => 1));
var_dump($status['ok']);
?>