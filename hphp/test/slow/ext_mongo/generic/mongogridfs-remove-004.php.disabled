<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = $db->getGridFS();
$gridfs->drop();
$id = $gridfs->storeFile(__FILE__);

$result = $gridfs->remove(array('_id' => $id), array('safe' => true));
var_dump((bool) $result['ok']);
var_dump(1 === $result['n']);
var_dump(null === $result['err']);