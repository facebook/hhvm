<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$m = new_mongo_standalone();
var_dump($m->setReadPreference(Mongo::RP_PRIMARY_PREFERRED, array(array('dc' => 'east'))));
var_dump($m->setReadPreference(Mongo::RP_PRIMARY_PREFERRED, array('not_an_array')));

?>