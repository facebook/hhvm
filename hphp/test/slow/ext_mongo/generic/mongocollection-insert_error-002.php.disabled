<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'insert');
$coll->drop();

$documents = array(
    array('_id' => "\xFE\xF0"),
    array('x' => "\xFE\xF0"),
    (object) array('x' => "\xFE\xF0"),
    array('x' => new MongoCode('return y;', array('y' => "\xFE\xF0"))),
);

foreach ($documents as $document) {
    try {
        $coll->insert($document);
    } catch (Exception $e) {
        printf("%s: %d\n", get_class($e), $e->getCode());
    }
}
?>