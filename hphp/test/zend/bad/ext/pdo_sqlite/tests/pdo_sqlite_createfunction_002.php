<?php

$db = new PDO( 'sqlite::memory:');

$db->sqliteCreateFunction('bar-alias', 'bar');

?>