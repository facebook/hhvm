<?php

$db = new SQLite3(':memory:');

$db->exec('CREATE TABLE test (whatever INTEGER)');
$db->exec('INSERT INTO test (whatever) VALUES (1)');

$result = $db->query('SELECT * FROM test');
while ($row = $result->fetchArray(SQLITE3_NUM)) {
    var_dump($result->columnName(0));  // string(8) "whatever"

    // Seems returning false will be most appropriate.
    var_dump($result->columnName(3));  // Segmentation fault
}

$result->finalize();
$db->close();

echo "Done\n";

?>
