<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();
$c = $m->phpunit->col;
var_dump($c->setSlaveOkay(array('error')));
?>