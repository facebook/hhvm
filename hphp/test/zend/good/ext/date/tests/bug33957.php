<?php
date_default_timezone_set("UTC");

for ($i = 14; $i <= 31; $i++) {
	echo "1992-12-$i  ", date("W", strtotime("1992-12-$i")), "\n";
}
for ($i = 1; $i <= 8; $i++) {
	echo "1993-01-$i  ", date("W", strtotime("1993-01-$i")), "\n";
}
echo "----\n";
echo "             ";
foreach (range(1992, 2019) as $year) {
	echo "$year     ";
}
echo "\n";

for ($i = 14; $i <= 31; $i++) {
	echo "   (12-$i) ";
	foreach (range(1992, 2019) as $year) {
		echo sprintf(" %02d-", date("W", strtotime("$year-12-$i")));
		echo sprintf("%04d ", date("o", strtotime("$year-12-$i")));
	}
	echo "\n";
}
for ($i = 1; $i <= 18; $i++) {
	printf("+1 (01-%02d) ", $i);
	foreach (range(1993, 2020) as $year) {
		echo sprintf(" %02d-", date("W", strtotime("$year-1-$i")));
		echo sprintf("%04d ", date("o", strtotime("$year-1-$i")));
	}
	echo "\n";
}
echo "----\n";
?>