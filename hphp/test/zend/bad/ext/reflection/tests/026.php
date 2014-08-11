<?php
$r = new ReflectionExtension("reflection");
$r->info();

date_default_timezone_set('Europe/Berlin');
$r = new ReflectionExtension("date");
$r->info();

echo "\nDone!\n";
?>
