<?php

if (date('T') == 'GMT') {
	putenv("TZ=EST5EDT4,M4.1.0,M10.5.0");
}
echo date("r\n", strtotime("Sep 04 16:39:45 2001"));
echo date("r\n", strtotime("Sep 04 2001 16:39:45"));	
?>