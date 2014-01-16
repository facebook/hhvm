<?php require_once __DIR__."/../utils/server.inc";

$mongo = new_mongo_standalone();
$database = $mongo->selectDB(dbname());

$collection = $database->selectCollection("test");
var_dump($collection);
$second_collection = $database->selectCollection($collection);
var_dump($second_collection);
var_dump($collection);

$collection->find();
?>
===DONE===
<?php exit(0);?>