<?php

print "TZ=America/Jujuy  - Is it OK for this to be 2 AM, rather than 1
AM as per most DST transitions?\n";
date_default_timezone_set("America/Jujuy");
$tStamp = mktime (17, 17, 17, 1, 7593, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";

print "TZ=Asia/Tbilisi - Is it OK for this to be 2 AM?\n";
date_default_timezone_set("Asia/Tbilisi");
$tStamp = mktime (17, 17, 17, 1, 12863, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday            00:00:00\n\n";
?>