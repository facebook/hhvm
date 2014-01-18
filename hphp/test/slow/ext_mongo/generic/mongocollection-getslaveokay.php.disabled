<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();

$c = $m->phpunit->col;
$c->setSlaveOkay((bool) array('foo'));
var_dump($c->getSlaveOkay());
$c->setSlaveOkay((bool) null);
var_dump($c->getSlaveOkay());
?>