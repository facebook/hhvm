<?php

require_once(dirname(__FILE__) . '/new_db.inc');

echo "SELECTING from invalid table\n";
$result = $db->query("SELECT * FROM non_existent_table");
if (!$result) {
	echo "Error Code: " . $db->lastErrorCode() . "\n";
	echo "Error Msg: " . $db->lastErrorMsg() . "\n";
}
echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
?>