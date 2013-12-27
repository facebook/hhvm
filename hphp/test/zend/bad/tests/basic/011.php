<?php
parse_str("ab+cd+ef+123+test", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();
 
$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

for ($i=0; $i<$argc; $i++) {
	echo "$i: ".$argv[$i]."\n";
}

?>