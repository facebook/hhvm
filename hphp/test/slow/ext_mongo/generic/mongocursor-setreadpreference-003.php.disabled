<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
$c = $m->phpunit->test->find();
var_dump($c === $c->setReadPreference(Mongo::RP_PRIMARY, array()));
var_dump($c->getReadPreference());
?>