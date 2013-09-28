<?php
date_default_timezone_set('Africa/Bujumbura');

print "TZ=Africa/Bujumbura - *Note*: Unexpected, as does not appear to
have a DST or timezone transition.\n";
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Wednesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Wednesday            00:00:00\n\n";

print "TZ=Asia/Thimbu - Is it OK for this to be 0:30 AM? yes\n";
date_default_timezone_set('Asia/Thimbu');
$tStamp = mktime (17, 17, 17, 1, 6476, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            00:30:00\n\n";

print "TZ=Indian/Cocos - Is it OK for this to be 6:30 AM? Note: does not
appear to have a DST or timezone transition.\n";
date_default_timezone_set('Indian/Cocos');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            00:00:00\n\n";

print "TZ=Africa/Lubumbashi - Is it OK for this to be 2 AM? Note: does
not appear to have a DST or timezone transition.\n";
date_default_timezone_set('Africa/Lubumbashi');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Saturday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Saturday            00:00:00\n\n";

print "TZ=Asia/Kashgar - Is it OK for this to be 3 AM? yes\n";
date_default_timezone_set('Asia/Kashgar');
$tStamp = mktime (17, 17, 17, 1, 3767, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            03:00:00\n\n";

print "TZ=Indian/Christmas - Is it OK for this to be 7 AM?  Note: does
not appear to have a DST or timezone transition.\n";
date_default_timezone_set('Indian/Christmas');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday            00:00:00\n\n";

print "TZ=America/Santo_Domingo - Is it OK for this to be 0:30 AM? yes\n";
date_default_timezone_set('America/Santo_Domingo');
$tStamp = mktime (17, 17, 17, 1, 291, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday            00:30:00\n\n";

print "TZ=Pacific/Truk - Is it OK for this to be 10 AM?  Note: does not
appear to have a DST or timezone transition.\n";
date_default_timezone_set('Pacific/Truk');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n";

print "TZ=Pacific/Ponape - Is it OK for this to be 11 AM?  Note: does
not appear to have a DST or timezone transition.\n";
date_default_timezone_set('Pacific/Ponape');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";

print "TZ=America/Scoresbysund - Is it OK for this to be 2 AM? yes\n";
date_default_timezone_set('America/Scoresbysund');
$tStamp = mktime (17, 17, 17, 1, 4099, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday            02:00:00\n\n";

print "TZ=America/Guyana - Is it OK for this to be 0:45 AM? yes\n";
date_default_timezone_set('America/Guyana');
$tStamp = mktime (17, 17, 17, 1, 2031, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            00:45:00\n\n";

print "TZ=Asia/Tehran - Is it OK for this to be 0:30 AM? yes\n";
date_default_timezone_set('Asia/Tehran');
$tStamp = mktime (17, 17, 17, 1, 2855, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:30:00\n\n";

print "TZ=Pacific/Tarawa - Is it OK for this to be Midday? Note: does
not appear to have a DST or timezone transition.\n";     
date_default_timezone_set('Pacific/Tarawa');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:00:00\n\n";

print "TZ=Africa/Monrovia - Is it OK for this to be 00:44:30 AM? yes\n";
date_default_timezone_set('Africa/Monrovia');
$tStamp = mktime (17, 17, 17, 1, 845, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:44:30\n\n";

print "TZ=Asia/Katmandu - Is it OK for this to 0:15 AM?. yes\n";
date_default_timezone_set('Asia/Katmandu');
$tStamp = mktime (17, 17, 17, 1, 5838, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Wednesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Wednesday            00:15:00\n\n";         

print "TZ=Pacific/Nauru - Is it OK for this to be 0:30? yes\n";
date_default_timezone_set('Pacific/Nauru');
$tStamp = mktime (17, 17, 17, 1, 3401, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:30:00\n\n";

print "TZ=Pacific/Niue - Is it OK for this to be 0:30 AM? yes\n";
date_default_timezone_set('Pacific/Niue');
$tStamp = mktime (17, 17, 17, 1, 3189, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Sunday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Sunday            00:30:00\n\n";            

print "TZ=Pacific/Port_Moresby - Is it OK for this to be 10 AM? No DST
or timezone transition.\n";
date_default_timezone_set('Pacific/Port_Moresby');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);   
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp); 
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            00:00:00\n\n";          

print "TZ=America/Miquelon - Is it OK for this to be 1 AM ? yes\n";
date_default_timezone_set('America/Miquelon');
$tStamp = mktime (17, 17, 17, 1, 3767, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Thursday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Thursday            01:00:00\n\n";

print "TZ=Pacific/Palau - Is it OK for this to be 9 AM? No DST or
timezone transition.\n";
date_default_timezone_set('Pacific/Palau');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);   
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Saturday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Saturday            00:00:00\n\n";

print "TZ=Pacific/Funafuti - Is it OK for this to be midday?  Note: does
not appear to have a DST or timezone transition.\n";
date_default_timezone_set('Pacific/Funafuti');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Wednesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Wednesday            00:00:00\n\n";

print "TZ=Pacific/Wake - Is it OK for this to be midday?  Note: does not
appear to have a DST or timezone transition.\n";
date_default_timezone_set('Pacific/Wake');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n";

print "TZ=Pacific/Wallis - Is it OK for this to be midday?  Note: does
not appear to have a DST or timezone transition.\n";
date_default_timezone_set('Pacific/Wallis');
$tStamp = mktime (17, 17, 17, 1, 1, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Tuesday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Tuesday            00:00:00\n\n";

print "TZ=America/Paramaribo - Is it OK for this to be 0:30 AM? yes\n";
date_default_timezone_set('America/Paramaribo');
$tStamp = mktime (17, 17, 17, 1, 5381, 1970);
print "tStamp=". date("l Y-m-d H:i:s T I", $tStamp). "\n";
$strtotime_tstamp = strtotime("next Monday", $tStamp);
print "result=".date("l Y-m-d H:i:s T I", $strtotime_tstamp)."\n";
print "wanted=Monday            00:30:00\n\n";

?>