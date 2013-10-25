<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$id = $gridfs->storeFile(__FILE__);

var_dump(0 < $gridfs->chunks->count());
var_dump(1 === $gridfs->count());

$gridfs->delete($id);

var_dump(0 === $gridfs->chunks->count());
var_dump(0 === $gridfs->count());