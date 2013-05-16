<?php

foreach (array(PREG_PATTERN_ORDER, PREG_SET_ORDER) as $flag) {
	var_dump(preg_match_all('~
		(?P<date> 
		(?P<year>(\d{2})?\d\d) -
		(?P<month>(?:\d\d|[a-zA-Z]{2,3})) -
		(?P<day>[0-3]?\d))
		~x',
		'2006-05-13 e outra data: "12-Aug-37"', $m, $flag));

	var_dump($m);
}
?>