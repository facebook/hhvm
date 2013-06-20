<?php

date_default_timezone_set("America/Los_Angeles");

$ts = mktime(0, 0, 0, 8, 5, 1998);

setlocale(2, LC_TIME, "C");
var_dump(strftime("%A", $ts));

if (setlocale(2, LC_TIME, "de_DE")) {
  var_dump(strftime(" in German %A.", $ts));
} else {
  //SKIP("setlocale() failed");
}

setlocale(2, LC_TIME, "C");

/*
December 2002 / January 2003
ISOWk  M   Tu  W   Thu F   Sa  Su
----- ----------------------------
51     16  17  18  19  20  21  22
52     23  24  25  26  27  28  29
1      30  31   1   2   3   4   5
2       6   7   8   9  10  11  12
3      13  14  15  16  17  18  19
*/
var_dump(strftime("%V,%G,%Y", strtotime("12/28/2002")));
var_dump(strftime("%V,%G,%Y", strtotime("12/30/2002")));
var_dump(strftime("%V,%G,%Y", strtotime("1/3/2003")));
var_dump(strftime("%V,%G,%Y", strtotime("1/10/2003")));

/*
December 2004 / January 2005
ISOWk  M   Tu  W   Thu F   Sa  Su
----- ----------------------------
51     13  14  15  16  17  18  19
52     20  21  22  23  24  25  26
53     27  28  29  30  31   1   2
1       3   4   5   6   7   8   9
2      10  11  12  13  14  15  16
*/
var_dump(strftime("%V,%G,%Y", strtotime("12/23/2004")));
var_dump(strftime("%V,%G,%Y", strtotime("12/31/2004")));
var_dump(strftime("%V,%G,%Y", strtotime("1/2/2005")));
var_dump(strftime("%V,%G,%Y", strtotime("1/3/2005")));
