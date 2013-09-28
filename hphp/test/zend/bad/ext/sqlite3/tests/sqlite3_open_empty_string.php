<?php
try{
    $db = new SQLite3('');
} catch(Exception $e) {
    echo $e->getMessage().PHP_EOL;
}
echo "Done\n";
?>