<?php

require_once(dirname(__FILE__) . '/new_db.inc');

$directory = dirname(__FILE__);

touch($directory . '/myext.txt');

var_dump($db->loadExtension('myext.txt'));
var_dump($db->close());
unlink($directory . '/myext.txt');

echo "Done\n";
?>