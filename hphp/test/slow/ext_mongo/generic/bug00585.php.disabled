<?php
require_once __DIR__."/../utils/server.inc";

$m = new_mongo_standalone();
var_dump(get_class($m));
$gridfs = $m->selectDb(dbname())->getGridFS();
var_dump($gridfs->w);


$m = mongo_standalone();
var_dump(get_class($m));
$gridfs = $m->selectDb(dbname())->getGridFS();

var_dump($gridfs->w);
?>