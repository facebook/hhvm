<?php
date_default_timezone_set("America/Toronto");

$t = mktime(0,0,0,1,1,292277026596); 

var_dump(date("r", $t)); 

echo "OK\n";
?>