<?php
require_once __DIR__."/../utils/server.inc";
$db = new MongoDB(mongo_standalone(), "phpunit");
$object = $db->selectCollection('c');
$object->drop();

$doc1 = array(
	'_id' => new MongoId('4cb4ab6d7addf98506010001'),
	'id' => 1,
	'desc' => "ONE",
);
$doc2 = array(
	'_id' => new MongoId('4cb4ab6d7addf98506010002'),
	'id' => 2,
	'desc' => "TWO",
);
$doc3 = array(
	'_id' => new MongoId('4cb4ab6d7addf98506010002'),
	'id' => 3,
	'desc' => "THREE",
);
$doc4 = array(
	'_id' => new MongoId('4cb4ab6d7addf98506010004'),
	'id' => 4,
	'desc' => "FOUR",
);

$object->batchInsert(array($doc1, $doc2, $doc3, $doc4), array('continueOnError' => true));

$c = $object->find();
foreach ($c as $item) {
	var_dump($item);
}
?>