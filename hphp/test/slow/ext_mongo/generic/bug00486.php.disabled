<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();
$db = $m->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$gridfs->storeFile("./README.md", array('_id' => 1));
$it = $gridfs->find();
foreach($it as $file) {
    var_dump($file->file["filename"]);
}
try {
    $gridfs->storeFile(__FILE__, array('_id' => 1));
} catch(MongoGridFSException $e) {
    var_dump($e->getMessage());
}
$it = $gridfs->find();
foreach($it as $file) {
    var_dump($file->file["filename"]);
}
$gridfs->drop();

?>