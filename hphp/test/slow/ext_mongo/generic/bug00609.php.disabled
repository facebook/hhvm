<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$gridfs = $m->selectDb(dbname())->getGridFS();
$retval = $gridfs->put(__FILE__, array("meta" => "data"));
$gridfs->drop();
var_dump($retval);
?>