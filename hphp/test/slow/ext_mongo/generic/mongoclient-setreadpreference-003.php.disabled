<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
var_dump($m->setReadPreference(Mongo::RP_PRIMARY, array()));
var_dump($m->getReadPreference());
?>