<?php

date_default_timezone_set("America/Los_Angeles");

$d = mktime(0, 0, 0, 1, 1, 1998);
var_dump(date("M d Y H:i:s",   $d));
var_dump(gmdate("M d Y H:i:s", $d));

$d = gmmktime(0, 0, 0, 1, 1, 1998);
var_dump(date("M d Y H:i:s",   $d));
var_dump(gmdate("M d Y H:i:s", $d));

setlocale(2, LC_TIME, "en_US.utf8");
$d = mktime(20, 0, 0, 12, 31, 98);
var_dump(strftime("%b %d %Y %H:%M:%S", $d));
var_dump(gmstrftime("%b %d %Y %H:%M:%S", $d));
$t = mktime(0,0,0, 6, 27, 2006);
var_dump(strftime("%a %A %b %B %c %C %d %D %e %g %G %h %H %I %j %m %M %n %p ".
              "%r %R %S %t %T %u %U %V %W %w %x %X %y %Y %Z %z %%", $t),
              "Tue Tuesday Jun June Tue 27 Jun 2006 12:00:00 AM PDT 20 27 ".
              "06/27/06 27 06 2006 Jun 00 12 178 06 00 \n AM 12:00:00 AM ".
              "00:00 00 \t 00:00:00 2 26 26 26 2 06/27/2006 12:00:00 AM ".
              "06 2006 PDT -0700 %");
