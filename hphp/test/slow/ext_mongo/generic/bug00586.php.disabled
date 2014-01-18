<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$gridfs->storeFile(__FILE__, array('x' => 1), array("w" => 1));

$file = $gridfs->findOne(array(), array('x' => 1));

try {
    $file->getBytes();
    var_dump(false);
} catch (MongoGridFSException $e) {
    var_dump(true);
}