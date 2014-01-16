<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

$gridfs = new MongoGridFS($db);
printf("%s\n", $gridfs);
printf("%s\n", $gridfs->chunks);