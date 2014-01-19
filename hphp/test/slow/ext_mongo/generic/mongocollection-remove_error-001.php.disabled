<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'remove');
$coll->drop();

try {
    $coll->remove(array('foo' => "\xFE\xF0"));
} catch (Exception $e) {
    printf("%s: %d\n", get_class($e), $e->getCode());
}
?>