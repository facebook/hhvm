<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();

$gridfs->storeFile(__FILE__);

$newFile = tempnam(sys_get_temp_dir(), 'mongogridfsfile-write');

$file = $gridfs->findOne();
var_dump(filesize(__FILE__) === $file->write($newFile));
var_dump(file_get_contents(__FILE__) === file_get_contents($newFile));

$gridfs->drop();
$gridfs->storeFile($newFile);

unlink($newFile);

$file = $gridfs->findOne();
var_dump(filesize(__FILE__) === $file->write());
var_dump(file_get_contents(__FILE__) === file_get_contents($newFile));