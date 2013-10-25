<?php

require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$grid = $m->selectDB(dbname())->getGridFS();
$id = $grid->storeBytes('foo');
$file = $grid->get($id);
try {
    $file->write();
} catch(MongoGridFSException $e) {
    var_dump($e->getMessage(), $e->getCode());
}
?>