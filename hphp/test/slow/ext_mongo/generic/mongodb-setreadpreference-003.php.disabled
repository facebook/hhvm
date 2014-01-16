<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
$db = $m->phpunit;
var_dump($db->setReadPreference(Mongo::RP_PRIMARY, array()));
var_dump($db->getReadPreference());
?>