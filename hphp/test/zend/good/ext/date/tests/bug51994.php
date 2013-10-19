<?php
$trans_date = '10153'; // 152nd day of year 2010 -> 03.06.2010
$a_date	= date_parse_from_format('yz', $trans_date);
var_dump($a_date);
?>