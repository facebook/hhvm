<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}

$ts = mktime(0, 0, 0, 8, 5, 1998);

setlocale(2, LC_TIME, "C");
VS(strftime("%A", $ts), "Wednesday");

if (setlocale(2, LC_TIME, "fi_FI")) {
  VS(strftime(" in Finnish is %A,", $ts), " in Finnish is keskiviikko,");
} else {
  //SKIP("setlocale() failed");
}

if (setlocale(2, LC_TIME, "fr_FR")) {
  VS(strftime(" in French %A and", $ts), " in French mercredi and");
} else {
  //SKIP("setlocale() failed");
}

if (setlocale(2, LC_TIME, "de_DE")) {
  VS(strftime(" in German %A.", $ts), " in German Mittwoch.");
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
VS(strftime("%V,%G,%Y", strtotime("12/28/2002")), "52,2002,2002");
VS(strftime("%V,%G,%Y", strtotime("12/30/2002")), "01,2003,2002");
VS(strftime("%V,%G,%Y", strtotime("1/3/2003")),   "01,2003,2003");
VS(strftime("%V,%G,%Y", strtotime("1/10/2003")),  "02,2003,2003");

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
VS(strftime("%V,%G,%Y", strtotime("12/23/2004")), "52,2004,2004");
VS(strftime("%V,%G,%Y", strtotime("12/31/2004")), "53,2004,2004");
VS(strftime("%V,%G,%Y", strtotime("1/2/2005")),   "53,2004,2005");
VS(strftime("%V,%G,%Y", strtotime("1/3/2005")),   "01,2005,2005");
