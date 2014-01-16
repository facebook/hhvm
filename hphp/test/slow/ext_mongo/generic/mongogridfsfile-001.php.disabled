<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();

$gridfs->storeFile(__FILE__);

$file = $gridfs->findOne();

var_dump($file->file['_id'] instanceof MongoId);
var_dump(__FILE__ === $file->file['filename']);
var_dump($file->file['uploadDate'] instanceof MongoDate);
var_dump(0 < $file->file['chunkSize']);

$contents = file_get_contents(__FILE__);

var_dump(strlen($contents) === $file->file['length']);
var_dump(md5($contents) === $file->file['md5']);

$gridfs->drop();
$gridfs->storeFile(__FILE__, array(
    '_id' => 1,
    'filename' => 'foo',
    'length' => 0,
    'chunkSize' => 10000,
    'uploadDate' => 'now',
    'md5' => 'f00',
));

$file = $gridfs->findOne();

var_dump(1 === $file->file['_id']);
var_dump('foo' === $file->file['filename']);
var_dump(10000 === $file->file['chunkSize']);
var_dump(0 === $file->file['length']);
var_dump('now' === $file->file['uploadDate']);
var_dump('f00' === $file->file['md5']);
