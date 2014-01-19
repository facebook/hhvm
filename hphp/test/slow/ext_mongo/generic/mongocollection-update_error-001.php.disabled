<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'update');
$coll->drop();

$coll->insert(array('_id' => 1, 'foo' => 'bar'));

try {
    $coll->update(array('_id' => 1), array('$set' => array('foo' => "\xFE\xF0")));
} catch (Exception $e) {
    printf("%s: %d\n", get_class($e), $e->getCode());
}

try {
    $coll->update(array('foo' => "\xFE\xF0"), array('$set' => array('foo' => 'bar')));
} catch (Exception $e) {
    printf("%s: %d\n", get_class($e), $e->getCode());
}
?>