<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n"; }
}

$d = mktime(0, 0, 0, 1, 1, 1998);
VS(date("M d Y H:i:s",   $d), "Jan 01 1998 00:00:00");
VS(gmdate("M d Y H:i:s", $d), "Jan 01 1998 08:00:00");

$d = gmmktime(0, 0, 0, 1, 1, 1998);
VS(date("M d Y H:i:s",   $d), "Dec 31 1997 16:00:00");
VS(gmdate("M d Y H:i:s", $d), "Jan 01 1998 00:00:00");

setlocale(2, LC_TIME, "en_US.utf8");
$d = mktime(20, 0, 0, 12, 31, 98);
VS(strftime("%b %d %Y %H:%M:%S", $d),   "Dec 31 1998 20:00:00");
VS(gmstrftime("%b %d %Y %H:%M:%S", $d), "Jan 01 1999 04:00:00");
$t = mktime(0,0,0, 6, 27, 2006);
VS(strftime("%a %A %b %B %c %C %d %D %e %g %G %h %H %I %j %m %M %n %p ".
              "%r %R %S %t %T %u %U %V %W %w %x %X %y %Y %Z %z %%", $t),
              "Tue Tuesday Jun June Tue 27 Jun 2006 12:00:00 AM PDT 20 27 ".
              "06/27/06 27 06 2006 Jun 00 12 178 06 00 \n AM 12:00:00 AM ".
              "00:00 00 \t 00:00:00 2 26 26 26 2 06/27/2006 12:00:00 AM ".
              "06 2006 PDT -0700 %");
