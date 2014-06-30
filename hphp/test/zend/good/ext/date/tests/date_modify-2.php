<?php
date_default_timezone_set("GMT");
$d = date_create("2005-07-18 22:10:00 +0400");
echo date_format($d, 'D, d M Y H:i:s T'), "\n";
date_modify($d, "+1 hour");
echo date_format($d, 'D, d M Y H:i:s T'), "\n";
?>