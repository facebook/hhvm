<?php

$dsn = 'sqlite:'.realpath(__DIR__).'/persistent.db';
$pdo = new PDO($dsn, null, null, array(PDO::ATTR_PERSISTENT => true));
var_dump('success');
