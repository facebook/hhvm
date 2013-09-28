<?php
	date_default_timezone_set("GMT");
	echo gmdate("F Y (Y-m-d H:i:s T)\n", strtotime("Oct 2001"));
	echo gmdate("M Y (Y-m-d H:i:s T)\n", strtotime("2001 Oct"));
?>