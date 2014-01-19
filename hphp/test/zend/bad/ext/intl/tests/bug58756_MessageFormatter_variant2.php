<?php
ini_set("intl.error_level", E_WARNING);
//ini_set("intl.default_locale", "nl");

$time = 1247013673;

ini_set('date.timezone', 'America/New_York');

$msgf = new MessageFormatter('en_US', '{0,date,full} {0,time,h:m:s a V}');

echo "date:  " . date('l, F j, Y g:i:s A T', $time) . "\n";
echo "msgf:  " . $msgf->format(array($time)) . "\n";

//NOT FIXED:
/*$msgf = new MessageFormatter('en_US',
'{1, select, date {{0,date,full}} other {{0,time,h:m:s a V}}}');

echo "msgf2: ", $msgf->format(array($time, 'date')), " ",
		$msgf->format(array($time, 'time')), "\n";
*/

?>
==DONE==