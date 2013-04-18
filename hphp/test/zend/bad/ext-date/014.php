<?php
date_default_timezone_set('UTC');

$dto = date_create("2006-12-12");
var_dump($dto);

$dtz = date_timezone_get($dto);
var_dump($dtz);

var_dump(timezone_offset_get());
var_dump(timezone_offset_get($dtz, $dto));
var_dump(timezone_offset_get($dto, $dtz));

echo "Done\n";

?>