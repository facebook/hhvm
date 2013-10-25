<?php
require_once __DIR__."/../utils/server.inc";

$m = new_mongo_standalone();
$gridfs = $m->selectDb(dbname())->getGridfs();
$gridfs->remove();

$id = $gridfs->put(__FILE__);
$coll = $m->selectDB(dbname())->selectCollection("fs.chunks");

// Fetch the first (only) chunk
$chunk = $coll->find(array("files_id" => $id))->getNext();
// Unset the id and bump the chunk# to create unexpectedly many chunks
unset($chunk["_id"]);
$chunk["n"]++;
$coll->insert($chunk);


// Now fetch the inserted file
$file = $gridfs->findOne(array("_id" => $id));

// Throws exception about to many chunks
try {
    $file->getBytes();
} catch(MongoGridFSException $e) {
    var_dump($e->getMessage(), $e->getCode());
}
?>