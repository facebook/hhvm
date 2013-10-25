<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$id = $gridfs->put(__FILE__, array('x' => 1));

$file = $gridfs->get($id);

var_dump(1 === $file->file['x']);