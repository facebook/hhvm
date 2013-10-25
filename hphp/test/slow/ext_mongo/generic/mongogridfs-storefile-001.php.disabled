<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$id = $gridfs->storeFile(__FILE__);

var_dump($id instanceof MongoId);

$file = $gridfs->findOne();
var_dump($id == $file->file['_id']);