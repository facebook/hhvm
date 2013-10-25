<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();

try {
    $gridfs->put('/does/not/exist');
    var_dump(false);
} catch (MongoGridFSException $e) {
    var_dump($e->getMessage(), $e->getCode());
}