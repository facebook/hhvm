<?php
date_default_timezone_set('Europe/London');

$start = new DateTime('2008-01-17 last Monday');
echo $start->format('Y-m-d H:i:s'),PHP_EOL;
//good

$start->modify('Tuesday');
echo $start->format('Y-m-d H:i:s'),PHP_EOL;
//good

$start->setTime(4, 0, 0);
echo $start->format('Y-m-d H:i:s'),PHP_EOL;
//jumped to next Sunday

$start->setTime(8, 0, 0);
echo $start->format('Y-m-d H:i:s'),PHP_EOL;
//jumped to next Sunday again
?>