<?php

$file = '/etc/passwd'.chr(0).'asdf';

$db = __DIR__ . DIRECTORY_SEPARATOR . 'example.db';

set_error_handler(function() use ($db) { unlink($db); return false; });

$sql3 = new SQLite3($db);
var_dump($sql3->loadExtension($file));

$sql3 = new SQlite3($file);
