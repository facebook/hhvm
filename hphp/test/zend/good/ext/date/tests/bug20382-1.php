<?php
	$tStamp = mktime (17, 17, 17, 10, 27, 2004);
	echo "tStamp=". date("l Y-m-d H:i:s T", $tStamp). "\n";
	
	$strtotime_timestamp = strtotime ("Monday", $tStamp);
	echo "result=". date("l Y-m-d H:i:s T", $strtotime_timestamp). "\n";
	echo "wanted=Monday 2004-11-01 00:00:00 CET\n";
?>