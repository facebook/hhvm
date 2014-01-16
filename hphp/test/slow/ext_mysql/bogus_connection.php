<?php
$bogus = 42;

var_dump(mysql_next_result($bogus));
var_dump(mysql_fetch_result($bogus));
var_dump(mysql_more_results($bogus));
var_dump(mysql_multi_query('SELECT 1;', $bogus));
