<?php
ini_set("intl.error_level", E_WARNING);
//ini_set("intl.default_locale", "nl");
ini_set('date.timezone', 'Europe/Lisbon');

$cal = new IntlGregorianCalendar(2012,04,17,17,35,36);

$msgf = new MessageFormatter('pt_PT', '{0,date,full} {0,time,h:m:s a V}');
echo $msgf->format(array($cal)), "\n";

//NOT FIXED:
/*$msgf = new MessageFormatter('en_US',
'{1, select, date {{0,date,full}} other {{0,time,h:m:s a V}}}');

echo "msgf2: ", $msgf->format(array($time, 'date')), " ",
		$msgf->format(array($time, 'time')), "\n";
*/

?>
==DONE==