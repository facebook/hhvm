<?php
date_default_timezone_set('Europe/Lisbon');
$time = 1150494719; // 16/June/2006

$strs = array(
	'',
	" \t\r\n000",
	'yesterday',
	'22:49:12',
	'22:49:12 bogusTZ',
	'22.49.12.42GMT',
	'22.49.12.42bogusTZ',
	't0222',
	't0222 t0222',
	'022233',
	'022233 bogusTZ',
	'2-3-2004',
	'2.3.2004',
	'20060212T23:12:23UTC',
	'20060212T23:12:23 bogusTZ',
	'2006167', //pgydotd
	'Jan-15-2006', //pgtextshort
	'2006-Jan-15', //pgtextreverse
	'10/Oct/2000:13:55:36 +0100', //clf
	'10/Oct/2000:13:55:36 +00100', //clf
	'2006',
	'1986', // year
	'JAN',
	'January',
);

foreach ($strs as $str) {
	$t = strtotime($str, $time);
	if (is_integer($t)) {
		var_dump(date(DATE_RFC2822, $t));
	} else {
		var_dump($t);
	}
}

?>