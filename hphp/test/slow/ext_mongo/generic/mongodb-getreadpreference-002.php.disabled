<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

// Set before MongoDB is instantiated
$m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_SECONDARY_PREFERRED));
$m->setReadPreference(Mongo::RP_PRIMARY_PREFERRED);
$db = $m->phpunit;
var_dump($db->getReadPreference());

// Set after MongoDB is instantiated
$m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_SECONDARY_PREFERRED));
$db = $m->phpunit;
$m->setReadPreference(Mongo::RP_SECONDARY);
var_dump($db->getReadPreference());
?>