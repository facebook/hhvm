<?php
date_default_timezone_set('UTC');

$dto = date_create("2006-12-12");
var_dump(date_isodate_set($dto, 2006, 2, 15));
var_dump($dto->format("Y/m/d H:i:s"));
var_dump(date_isodate_set($dto, 2006));
var_dump($dto->format("Y/m/d H:i:s"));
var_dump(date_isodate_set($dto, 2006, 5));
var_dump($dto->format("Y/m/d H:i:s"));
var_dump(date_isodate_set($dto, 2006, 100, 15));
var_dump($dto->format("Y/m/d H:i:s"));
var_dump(date_isodate_set($dto, 2006, 100, 15, 10));
var_dump($dto->format("Y/m/d H:i:s"));

echo "Done\n";
?>