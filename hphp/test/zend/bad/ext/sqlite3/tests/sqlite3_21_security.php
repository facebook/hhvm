<?php
$directory = dirname(__FILE__) . '/';
$file = uniqid() . '.db';

echo "Within test directory\n";
$db = new SQLite3($directory . $file);
var_dump($db);
var_dump($db->close());
unlink($directory . $file);

echo "Above test directory\n";
try {
	$db = new SQLite3('../bad' . $file);
} catch (Exception $e) {
	echo $e . "\n";
}

echo "Done\n";
?>