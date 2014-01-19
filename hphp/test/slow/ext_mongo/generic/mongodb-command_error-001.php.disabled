<?php
require __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$db = $m->selectDb('phpunit');
$retval = $db->command(array());
var_dump($retval["errmsg"], $retval["bad cmd"], $retval["ok"]);
?>