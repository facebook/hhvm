<?php
require_once __DIR__."/../utils/server.inc";

$cfg = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($cfg["dsn"], array("replicaSet" => $cfg["rsname"]));

$mc->selectCollection("test", "PHP-698")->drop();
for($i = 0; $i< 512; $i++) {
    $mc->selectCollection("test", "PHP-698")->insert(array("foo" => "bar", "i" => $i));
}

$c = $mc->selectCollection("test", "PHP-698")->find();
$n = 0;
// Leave an unfinished cursor open
foreach($c as $doc) {
    if ($n++ > 100) break;
}

$mc->selectCollection("test", "PHP-698")->drop();

// Closing a connection with a cursor open segfaults
$mc->close();
echo "I should still be alive!\n";
?>