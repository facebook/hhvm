<?php
require_once __DIR__."/../../utils/server.inc";
// Connect to mongo
$m = mongo_standalone();
$collection = $m->selectCollection(dbname(), 'crash');
$collection->drop();

// Load the problem record.
$arrayData = file_get_contents("tests/data-files/bug00485-data.txt");
$doc = unserialize($arrayData);

$collection->insert( $doc );

$doc = $collection->findOne(array('_id' => new MongoId('4ffe06d19da778b67809666a')));

$collection->update(array('_id' => $doc['_id']), array('$set' => array('image.id' => new MongoId('50470e396e6adf8f4a000039'))));
$res = $collection->save($doc, array('safe' => 1));
var_dump($res);
echo "DONE\n";
?>