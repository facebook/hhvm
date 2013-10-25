<?php

require_once __DIR__."/../../utils/server.inc";
$cfg = MongoShellServer::getStandaloneInfo();

$m = new Mongo($cfg, array("connect" => false));
var_dump($m, $m->connected);

$m = new Mongo($cfg);
var_dump($m, $m->connected);
?>