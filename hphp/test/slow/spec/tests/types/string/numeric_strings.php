<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

var_dump(setlocale(LC_ALL, 0));
var_dump(setlocale(LC_NUMERIC, "fr-CA"));
var_dump(setlocale(LC_NUMERIC, "fr-BE"));
var_dump(setlocale(LC_NUMERIC, "fr-CH"));
var_dump(setlocale(LC_NUMERIC, "fr-FR"));
// var_dump(setlocale(LC_NUMERIC, "XXX"));  // returns False as there is no such locale on system

$s = vec[
    "",
    "0", "00", "0377", "0xEEFFAA00", "0X1234EF",
    " 0", "  00", "   0377", "    0xEEFFAA00", "     0X1234EF",
    "0 ", "00  ", "0377   ", "0xEEFFAA00    ", "0X1234EF     ",
    "0b1010", "0B111111111111111",
    "+0", "+1234567890", "-187654321",
    "123.456", "-7654.", ".7654321",
    "1e12", "+2E21", "-4e+2", "9E-21",
    "-123.762e21", "+876.432E37",
    "INF", "INf", "InF", "Inf", "iNF", "iNf", "inF", "inf",
    "+INF", "+INf", "+InF", "+Inf", "+iNF", "+iNf", "+inF", "+inf",
    "-INF", "-INf", "-InF", "-Inf", "-iNF", "-iNf", "-inF", "-inf",
    "NAN", "NAn", "NaN", "Nan", "nAN", "nAn", "naN", "nan"
];

foreach ($s as $e) {
    echo ">$e< is ".(is_numeric($e) ? "numeric\n" : "not numeric\t\t***\n");
}

try {
  sprintf($t, ",%b,%B", 0b1010, 0b1010);
  var_dump($t);
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
var_dump((string)INF);
var_dump((string)-INF);
var_dump((string)NAN);
}
