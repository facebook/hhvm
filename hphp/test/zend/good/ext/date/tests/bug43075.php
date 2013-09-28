<?php
date_default_timezone_set('UTC');
$d = date_create("2007-11-01T24:34:00+00:00");
echo $d->format("c");
?>