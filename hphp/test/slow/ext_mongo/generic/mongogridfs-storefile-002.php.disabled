<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$id = $gridfs->storeFile(__FILE__, array('_id' => 1));

var_dump(1 === $id);

$file = $gridfs->findOne();
var_dump($id == $file->file['_id']);