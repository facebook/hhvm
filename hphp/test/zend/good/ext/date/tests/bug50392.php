<?php
date_default_timezone_set('Europe/Bratislava');

$base = '2009-03-01 18:00:00';

for ($i = 0; $i < 8; $i++) {
	$string = $base . '.' . str_repeat($i, $i);
	echo $string, "\n- ";
	$result = date_parse_from_format('Y-m-d H:i:s.u', $string);
	echo $result['fraction'] ? $result['fraction'] : 'X', "\n";
	foreach( $result['errors'] as $error ) {
		echo "- ", $error, "\n";
	}
	echo "\n";
}
?>