<?php

$db = new SQLite3(':memory:');
var_dump($db->enableExceptions(true));
try{
    $db->query("SELECT * FROM non_existent_table");
} catch(Exception $e) {
    echo $e->getMessage().PHP_EOL;
}
var_dump($db->enableExceptions(false));
$db->query("SELECT * FROM non_existent_table");
var_dump($db->enableExceptions("wrong_type","wrong_type"));
echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
?>