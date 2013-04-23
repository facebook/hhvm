<?php

foreach (array('2006-05-13', '06-12-12', 'data: "12-Aug-87"') as $s) {
	var_dump(preg_match('~
		(?P<date> 
		(?P<year>(\d{2})?\d\d) -
		(?P<month>(?:\d\d|[a-zA-Z]{2,3})) -
		(?P<day>[0-3]?\d))
	~x', $s, $m));

	var_dump($m);
}

?>