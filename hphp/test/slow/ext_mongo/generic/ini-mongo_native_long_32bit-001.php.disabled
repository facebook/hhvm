<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongo_native_long');
$coll->drop();

ini_set('mongo.long_as_object', false);
ini_set('mongo.native_long', true);

$coll->insert(array('int64' => new MongoInt64('9223372036854775807')));

try {
    $coll->findOne();
} catch (Exception $e) {
    printf("%s: %s\n", get_class($e), $e->getMessage());
}
?>