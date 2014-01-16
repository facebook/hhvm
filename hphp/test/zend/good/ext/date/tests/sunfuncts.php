<?php
date_default_timezone_set('Asia/Jerusalem');
for($a=1;$a<=12;$a++){
	echo date_sunrise(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_TIMESTAMP,31.76670,35.23330,90.83,2)." ";
	echo date_sunrise(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_STRING,31.76670,35.23330,90.83,2)." ";
	echo date_sunrise(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_DOUBLE,31.76670,35.23330,90.83,2)."\n";
	
	echo date_sunset(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_TIMESTAMP,31.76670,35.23330,90.83,2)." ";
	echo date_sunset(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_STRING,31.76670,35.23330,90.83,2)." ";
	echo date_sunset(mktime(1,1,1,$a,1,2003),SUNFUNCS_RET_DOUBLE,31.76670,35.23330,90.83,2)."\n";
}
?>