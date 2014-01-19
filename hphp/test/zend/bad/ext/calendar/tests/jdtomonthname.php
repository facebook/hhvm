<?php

$jd_days = Array(
    2453396,
    2440588,
    -1,
    array(),
    10000000
    );

foreach ($jd_days as $jd_day) {
	echo "=== ", $jd_day, "\n";
    var_dump(jdmonthname($jd_day,0));
    var_dump(jdmonthname($jd_day,1));
    var_dump(jdmonthname($jd_day,2));
    var_dump(jdmonthname($jd_day,3));
    var_dump(jdmonthname($jd_day,4));
    var_dump(jdmonthname($jd_day,5));
	echo "\n";
}

echo "Done\n";

?>