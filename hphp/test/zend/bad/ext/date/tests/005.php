<?php
date_default_timezone_set('UTC');

$t = mktime(0,0,0, 6, 27, 2006);

var_dump(idate());
var_dump(idate(1,1,1));

var_dump(idate(1,1));
var_dump(idate(""));
var_dump(idate(0));

var_dump(idate("B", $t));
var_dump(idate("[", $t));
var_dump(idate("'"));

echo "Done\n";
?>