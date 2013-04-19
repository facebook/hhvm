<?php
print "TZ=Pacific/Rarotonga - wrong day.\n";
date_default_timezone_set("Pacific/Rarotonga");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);       
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n"; 

print "TZ=Atlantic/South_Georgia - wrong day.\n";
date_default_timezone_set("Atlantic/South_Georgia");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n";

print "TZ=America/Port-au-Prince - wrong day.\n";
date_default_timezone_set("America/Port-au-Prince");
$tStamp = mktime (17, 17, 17, 1, 12871, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";

print "TZ=Pacific/Enderbury - wrong day, off by 2 days.\n";
date_default_timezone_set("Pacific/Enderbury");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";

print "TZ=Pacific/Kiritimati - wrong day, off by 2 days.\n";
date_default_timezone_set("Pacific/Kiritimati");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);                 
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";

print "TZ=America/Managua - wrong day.\n";     
date_default_timezone_set("America/Managua");
$tStamp = mktime (17, 17, 17, 1, 12879, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n";     

print "TZ=Pacific/Pitcairn - wrong day.\n";
date_default_timezone_set("Pacific/Pitcairn");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Wednesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Wednesday            00:00:00\n\n";

print "TZ=Pacific/Fakaofo - wrong day.\n";
date_default_timezone_set("Pacific/Fakaofo");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Saturday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Saturday            00:00:00\n\n";

print "TZ=Pacific/Johnston - wrong day.\n";
date_default_timezone_set("Pacific/Johnston");
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Friday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Friday            00:00:00\n\n";
?>