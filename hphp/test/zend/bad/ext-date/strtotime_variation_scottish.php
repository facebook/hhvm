<?php
	date_default_timezone_set('UTC');
	var_dump(date('H:i:s', strtotime('back of 7')));
	var_dump(date('H:i:s', strtotime('front of 7')));
	var_dump(date('H:i:s', strtotime('back of 19')));
	var_dump(date('H:i:s', strtotime('front of 19')));
?>