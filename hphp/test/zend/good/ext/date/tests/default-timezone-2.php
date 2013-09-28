<?php
date_default_timezone_set('Europe/Oslo');
	putenv('TZ='); // clean TZ so that it doesn't bypass the ini option
	echo strtotime("2005-06-18 22:15:44");
?>