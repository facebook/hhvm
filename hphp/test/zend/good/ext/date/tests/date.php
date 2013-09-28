<?php
$tmp = "cr";
date_default_timezone_set('UTC');

for($a = 0;$a < strlen($tmp); $a++){
	echo $tmp[$a], ': ', date($tmp[$a], 1043324459)."\n";
}

date_default_timezone_set("MET");

for($a = 0;$a < strlen($tmp); $a++){
	echo $tmp[$a], ': ', date($tmp[$a], 1043324459)."\n";
}
?>