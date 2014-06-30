<?php
ini_set('date.timezone', 'UTC');

print "Date         PHP\n";
print "----------   ---\n";
$dates = array('1599-12-30', '1599-12-31', '1600-01-01', '1600-01-02');
foreach ($dates as $date) {
	echo date_create($date)->format('Y-m-d   D'), "\n";
}
?>