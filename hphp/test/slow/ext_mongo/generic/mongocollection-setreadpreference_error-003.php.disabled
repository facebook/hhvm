<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
$c = $m->phpunit->test;
$c->setReadPreference(Mongo::RP_PRIMARY, array( array( 'foo' => 'bar' ) ) );
$rp = $c->getReadPreference();
var_dump($rp);
?>