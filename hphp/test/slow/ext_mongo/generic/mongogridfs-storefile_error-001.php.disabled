<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$gridfs->storeFile(__FILE__, array('_id' => 1), array('safe' => true));

try {
    $gridfs->storeFile(__FILE__, array('_id' => 1), array('safe' => true));
    var_dump(false);
} catch (MongoGridFSException $e) {
    var_dump($e->getMessage(), $e->getCode());
}