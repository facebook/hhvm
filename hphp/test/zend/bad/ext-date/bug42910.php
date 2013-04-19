<?php
	date_default_timezone_set('America/Los_Angeles');
	$foo = new DateTime('2007-03-11');
	$bar = new DateTime('2007-03-11T00:00:00-0800');

	print $foo->format(DateTime::ISO8601) . ' - ' .  $foo->getTimezone()->getName() . ' - ' . $foo->format('U') . "\r\n";
	print $bar->format(DateTime::ISO8601) . ' - ' .  $bar->getTimezone()->getName() . ' - ' . $bar->format('U') . "\r\n";

	$foo->setDate(2007, 03, 12);
	$bar->setDate(2007, 03, 12);

	print $foo->format(DateTime::ISO8601) . ' - ' .  $foo->getTimezone()->getName() . ' - ' . $foo->format('U') . "\r\n";
	print $bar->format(DateTime::ISO8601) . ' - ' .  $bar->getTimezone()->getName() . ' - ' . $bar->format('U') . "\r\n";

// --------------

    date_default_timezone_set('Australia/Sydney');
  
    $date= date_create('2007-11-04 12:00:00+0200');
    var_dump(date_format($date, 'O e'));
?>