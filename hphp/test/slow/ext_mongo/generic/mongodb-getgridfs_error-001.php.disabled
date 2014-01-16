<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB(dbname());

try {
    $db->getGridFS(null);
    var_dump(false);
} catch (Exception $e) {
    var_dump($e->getMessage(), $e->getCode());
}