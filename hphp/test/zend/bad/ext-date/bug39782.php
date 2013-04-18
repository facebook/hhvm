<?php
date_default_timezone_set("GMT");

$dttTest = new DateTime('Dec 10 2006 Next Wednesday');
echo $dttTest->format('D M j Y - H:i:s') . "\n";

$dttTest->setTime(12, 0, 0);
echo $dttTest->format('D M j Y - H:i:s') . "\n";

$dttTest->setTime(12, 0, 0);
echo $dttTest->format('D M j Y - H:i:s') . "\n";
?>