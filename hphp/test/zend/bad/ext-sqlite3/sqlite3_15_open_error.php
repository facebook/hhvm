<?php
$unreadable = __DIR__ . '/unreadable.db';
touch($unreadable);
chmod($unreadable,  0200);
try {
	$db = new SQLite3($unreadable);
} catch (Exception $e) {
	echo $e . "\n";
}
echo "Done\n";
unlink($unreadable);
?>