<?php

date_default_timezone_set('America/New_York');
// PHP defaults to Daylight Saving Time.  Ensure consistency in future.
$d = new DateTime('2011-11-06 01:30:00');
echo $d->format('P') . "\n";

?>
