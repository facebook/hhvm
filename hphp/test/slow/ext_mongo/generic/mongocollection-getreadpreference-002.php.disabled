<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

// Set before MongoCollection is instantiated
$m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_SECONDARY_PREFERRED));
$db = $m->phpunit;
$db->setReadPreference(Mongo::RP_PRIMARY_PREFERRED);
$c = $db->test;
var_dump($c->getReadPreference());

// Set after MongoCollection is instantiated
$m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_SECONDARY_PREFERRED));
$db = $m->phpunit;
$c = $db->test;
$db->setReadPreference(Mongo::RP_SECONDARY);
var_dump($c->getReadPreference());
?>