<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
$m->setReadPreference(MongoClient::RP_PRIMARY, array( array( 'foo' => 'bar' ) ) );
$rp = $m->getReadPreference();
var_dump($rp);
?>