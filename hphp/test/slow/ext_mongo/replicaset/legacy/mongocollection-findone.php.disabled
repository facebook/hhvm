<?php
require_once __DIR__."/../../utils/server.inc";
$cfg = MongoShellServer::getReplicasetInfo();

$mongoConnection = new Mongo($cfg["hosts"][0] . "," . $cfg["hosts"][1], array('replicaSet' => rsname()));
$dbname = dbname();
$db = $mongoConnection->$dbname;
$db->setSlaveOkay();

for ($i = 0; $i < 50; $i++) {
	$user = $db->users->findOne(array('email.address' => 'not-my-email@example.com'));
	var_dump($user);
}
?>