<?php

$file = '/etc/passwd'.chr(0).'asdf';

$sql3 = new SQLite3('example.db');
var_dump($sql3->loadExtension($file));

$sql3 = new SQlite3($file);
