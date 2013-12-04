<?php 

print "TZ=America/Mendoza - wrong day.\n";
$tStamp = mktime (17, 17, 17, 1, 8327, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday              00:00:00\n\n";

print "TZ=America/Catamarca - wrong day.\n";
date_default_timezone_set("America/Catamarca");
$tStamp = mktime (17, 17, 17, 1, 7599, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday              00:00:00\n\n";

print "TZ=America/Cordoba - wrong day.\n";
date_default_timezone_set("America/Cordoba");
$tStamp = mktime (17, 17, 17, 1, 7599, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday              00:00:00\n\n";

print "TZ=America/Rosario - wrong day.\n";
date_default_timezone_set("America/Rosario");
$tStamp = mktime (17, 17, 17, 1, 7958, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday           00:00:00\n\n";

print "TZ=Europe/Vienna - wrong day - giving unexpected results, at
least on my system :-)\n";
date_default_timezone_set("Europe/Vienna");
$tStamp = mktime (17, 17, 17, 1, 3746, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday             00:00:00\n\n";

print "TZ=Asia/Baku - wrong day.\n";
date_default_timezone_set("Asia/Baku");
$tStamp = mktime (17, 17, 17, 1, 8299, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday              00:00:00\n\n";

print "TZ=America/Noronha - wrong day.\n";
date_default_timezone_set("America/Noronha");
$tStamp = mktime (17, 17, 17, 1, 10866, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Friday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Friday              00:00:00\n\n";

print "TZ=America/Havana - wrong day.\n";
date_default_timezone_set("America/Havana");
$tStamp = mktime (17, 17, 17, 1, 12720, 1970);  
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday             00:00:00\n\n";

print "TZ=Europe/Tallinn - wrong day.\n";
date_default_timezone_set("Europe/Tallinn");   
$tStamp = mktime (17, 17, 17, 1, 11777, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Saturday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Saturday          00:00:00\n\n";  

print "TZ=Asia/Jerusalem - wrong day.\n";     
date_default_timezone_set("Asia/Jerusalem");
$tStamp = mktime (17, 17, 17, 1, 13056, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday             00:00:00\n\n";         

print "TZ=Europe/Vilnius - wrong day.\n";
date_default_timezone_set("Europe/Vilnius");
$tStamp = mktime (17, 17, 17, 1, 12140, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Friday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Friday            00:00:00\n\n";

print "TZ=Pacific/Kwajalein - wrong day.\n";
date_default_timezone_set("Pacific/Kwajalein");
$tStamp = mktime (17, 17, 17, 1, 8626, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Friday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Friday            00:00:00\n\n";

print "TZ=Asia/Ulan_Bator - wrong day.\n";
date_default_timezone_set("Asia/Ulan_Bator");
$tStamp = mktime (17, 17, 17, 1, 11588, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Saturday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Saturday            00:00:00\n\n";

print "TZ=America/Cancun - wrong day.\n";
date_default_timezone_set("America/Cancun");
$tStamp = mktime (17, 17, 17, 1, 11785, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday            00:00:00\n\n";

print "TZ=America/Mexico_City - wrong day.\n";
date_default_timezone_set("America/Mexico_City");
$tStamp = mktime (17, 17, 17, 1, 11781, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Wednesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Wednesday            00:00:00\n\n";

print "TZ=America/Mazatlan - wrong day.\n";
date_default_timezone_set("America/Mazatlan");
$tStamp = mktime (17, 17, 17, 1, 11780, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n";

print "TZ=America/Chihuahua - wrong day.\n";
date_default_timezone_set("America/Chihuahua");
$tStamp = mktime (17, 17, 17, 1, 11782, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            00:00:00\n\n";

print "TZ=Asia/Kuala_Lumpur - wrong day.\n";     
date_default_timezone_set("Asia/Kuala_Lumpur");
$tStamp = mktime (17, 17, 17, 1, 4380, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";            

print "TZ=Pacific/Chatham - wrong day.\n";       
date_default_timezone_set("Pacific/Chatham");  
$tStamp = mktime (17, 17, 17, 1, 1762, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";            

print "TZ=America/Lima - wrong day.\n";        
date_default_timezone_set("America/Lima");   
$tStamp = mktime (17, 17, 17, 1, 5839, 1970); 
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            00:00:00\n\n";          

print "TZ=Asia/Karachi - wrong day.\n";
date_default_timezone_set("Asia/Karachi");
$tStamp = mktime (17, 17, 17, 1, 11783, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Friday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Friday            00:00:00\n\n";

print "TZ=America/Asuncion - wrong day.\n";
date_default_timezone_set("America/Asuncion");
$tStamp = mktime (17, 17, 17, 1, 11746, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Wednesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Wednesday            00:00:00\n\n";

print "TZ=Asia/Singapore - wrong day.\n";
date_default_timezone_set("Asia/Singapore");
$tStamp = mktime (17, 17, 17, 1, 4383, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday             00:00:00\n\n";

print "TZ=America/Montevideo - wrong day.\n";
date_default_timezone_set("America/Montevideo");
$tStamp = mktime (17, 17, 17, 1, 12678, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday             00:00:00\n\n";

?>