<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$gridfs->storeFile(__FILE__);

$cursor = $gridfs->find();
var_dump($cursor instanceof MongoGridFSCursor);
var_dump($cursor->getNext() instanceof MongoGridFSFile);
var_dump(null === $cursor->getNext());