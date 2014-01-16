<?php
require_once __DIR__."/../utils/server.inc";
$m = new_mongo_standalone();
$db = $m->phpunit;

$db->people->drop();
$db->people->ensureIndex(array('title' => true), array('sparse' => true));
$db->people->insert(array('name' => 'Jim'));
$db->people->insert(array('name' => 'Bones', 'title' => 'Doctor'));

foreach ($db->people->find() as $r) {
	echo @"Name: {$r['name']}; Title: {$r['title']}\n";
}
echo "\n";

foreach ($db->people->find()->sort(array('title' => 1)) as $r) {
	echo @"Name: {$r['name']}; Title: {$r['title']}\n";
}
echo "\n";

$db->people->deleteIndex(array('title' => true));

foreach ($db->people->find()->sort(array('title' => 1)) as $r) {
	echo @"Name: {$r['name']}; Title: {$r['title']}\n";
}
echo "\n";

$db->people->drop();
?>