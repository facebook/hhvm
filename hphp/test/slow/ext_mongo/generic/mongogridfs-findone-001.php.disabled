<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();

var_dump(null === $gridfs->findOne());

$gridfs->storeFile(__FILE__);

var_dump($gridfs->findOne() instanceof MongoGridFSFile);