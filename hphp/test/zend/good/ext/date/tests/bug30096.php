<?php
echo "no dst --> dst\n";
$ts = -1;
gm_date_check(01,00,00,03,27,2005);
gm_date_check(02,00,00,03,27,2005);
gm_date_check(03,00,00,03,27,2005);
gm_date_check(04,00,00,03,27,2005);

echo "\ndst --> no dst\n";
$ts = -1;
gm_date_check(01,00,00,10,30,2005);
gm_date_check(02,00,00,10,30,2005);
gm_date_check(03,00,00,10,30,2005);
gm_date_check(04,00,00,10,30,2005);

function gm_date_check($hour, $minute, $second, $month, $day, $year) {
	global $ts, $tsold;

	echo "gmmktime($hour,$minute,$second,$month,$day,$year): ";
	
	$tsold = $ts;
	$ts = gmmktime($hour, $minute, $second, $month, $day, $year);

	echo $ts, " | gmdate('r', $ts):", gmdate('r', $ts);
	if ($tsold > 0) {
		echo " | Diff: " . ($ts - $tsold);
	}
	echo "\n";
}

?>