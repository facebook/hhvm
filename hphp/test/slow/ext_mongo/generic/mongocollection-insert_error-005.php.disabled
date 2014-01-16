<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'insert');
$coll->drop();

try {
    $coll->insert(array('x.y' => 'z'));
} catch (Exception $e) {
    printf("%s: %d\n", get_class($e), $e->getCode());
}
?>