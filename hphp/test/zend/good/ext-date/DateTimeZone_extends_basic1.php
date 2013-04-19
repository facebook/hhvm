<?php

//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing basic DateTimeZone inheritance() ***\n";

class DateTimeZoneExt extends DateTimeZone
{
	public function __toString()
	{
		return parent::getName();
	}
}

echo "\n-- Create an instance of DateTimeZoneExt --\n";
$d = new DateTimeZoneExt("America/Los_Angeles");

echo "\n-- Invoke __toString --\n";
echo $d . "\n";

?>
===DONE===