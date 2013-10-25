<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

// Set before MongoCursor is instantiated
$m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_SECONDARY_PREFERRED));
$col = $m->phpunit->test;
$col->setReadPreference(Mongo::RP_PRIMARY_PREFERRED);
$c = $col->find();
var_dump($c->getReadPreference());

// Set after MongoCursor is instantiated
$m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_SECONDARY_PREFERRED));
$col = $m->phpunit->test;
$c = $col->find();
$col->setReadPreference(Mongo::RP_PRIMARY_PREFERRED);
var_dump($c->getReadPreference());
?>