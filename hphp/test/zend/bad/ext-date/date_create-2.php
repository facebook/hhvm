<?php
date_default_timezone_set("GMT");
$d = date_create("2005-07-18 22:10:00 +0400");
echo $d->format('D, d M Y H:i:s T'), "\n";

$d = date_create("@1121710200 +0912");
echo $d->format('D, d M Y H:i:s T'), "\n";
?>