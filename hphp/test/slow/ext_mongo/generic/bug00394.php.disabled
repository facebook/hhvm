<?php
require_once __DIR__."/../utils/server.inc";

$x = new Mongo(array());
var_dump($x);
if ($x) {
    $x->connect();
}
$x = new MongoDB;
var_dump($x);
if ($x) {
    $x->dropCollection(NULL);
}

$x = new MongoDB;
var_dump($x);
if ($x) {
    $x->listCollections();
}

$x = new MongoDB;
var_dump($x);
if ($x) {
    $x->getCollectionNames();
}

$x = new MongoGridFS;
var_dump($x);
if ($x) {
    $x->drop();
}

$x = new MongoGridFSFile;
var_dump($x);
if ($x) {
    $x->getFilename(-1);
}


?>