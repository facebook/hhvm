<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
$db = $m->phpunit;
$db->setReadPreference(MongoClient::RP_SECONDARY, array( array( 'foo' => 'bar' ) ) );
$db->setReadPreference(MongoClient::RP_PRIMARY, array( array( 'foo' => 'bar' ) ) );
$rp = $db->getReadPreference();
var_dump($rp);
?>