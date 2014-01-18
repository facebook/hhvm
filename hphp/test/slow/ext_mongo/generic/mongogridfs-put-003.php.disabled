<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();

$gridfs->put('tests/data-files/empty');

$file = $gridfs->findOne();

var_dump(0 === $file->file['length']);