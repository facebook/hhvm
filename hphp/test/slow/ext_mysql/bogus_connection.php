<?php
$bogus = 42;

mysql_next_result($bogus);
mysql_fetch_result($bogus);
mysql_more_results($bogus);
mysql_multi_query('SELECT 1;', $bogus);
