<?php
date_default_timezone_set("Pacific/Kwajalein");
$ts = date_create("Thu Aug 19 1993 23:59:59");
echo date_format($ts, 'D, d M Y H:i:s T'), "\n";
$ts->modify("+1 second");
echo date_format($ts, 'D, d M Y H:i:s T'), "\n";

date_default_timezone_set("Europe/Amsterdam");
$ts = date_create("Sun Mar 27 01:59:59 2005");
echo date_format($ts, 'D, d M Y H:i:s T'), "\n";
$ts->modify("+1 second");
echo date_format($ts, 'D, d M Y H:i:s T'), "\n";

$ts = date_create("Sun Oct 30 01:59:59 2005");
echo date_format($ts, 'D, d M Y H:i:s T'), "\n";
$ts->modify("+ 1 hour 1 second");
echo date_format($ts, 'D, d M Y H:i:s T'), "\n";
?>
