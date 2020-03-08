<?hh // strict

namespace NS_numeric_strings;

function main(): void {
/* All LC_* names are unbound

  var_dump(setlocale(LC_ALL, 0));
  var_dump(setlocale(LC_NUMERIC, "fr-CA"));
  var_dump(setlocale(LC_NUMERIC, "fr-BE"));
  var_dump(setlocale(LC_NUMERIC, "fr-CH"));
  var_dump(setlocale(LC_NUMERIC, "fr-FR"));
//  var_dump(setlocale(LC_NUMERIC, "XXX"));	// returns False as there is no such locale on system
*/

$s = array(
  "",
  "0", "00", "0377", "0xEEFFAA00", "0X1234EF",
  " 0", "  00", "   0377", "    0xEEFFAA00", "     0X1234EF",
  "0 ", "00  ", "0377   ", "0xEEFFAA00    ", "0X1234EF     ",
  "0b1010", "0B111111111111111",
  "+0", "+01234567890", "-187654321",
  "123.456", "-7654.", ".7654321",
  "1e12", "+2E21", "-4e+2", "9E-21",
  "-123.762e21", "+876.432E37",
  "INF", "INf", "InF", "Inf", "iNF", "iNf", "inF", "inf",
  "+INF", "+INf", "+InF", "+Inf", "+iNF", "+iNf", "+inF", "+inf",
  "-INF", "-INf", "-InF", "-Inf", "-iNF", "-iNf", "-inF", "-inf",
  "NAN", "NAn", "NaN", "Nan", "nAN", "nAn", "naN", "nan"
);
  foreach ($s as $e) {
    if (is_numeric($e)) {
      echo "\"$e\" is numeric; value is " . (float)$e . "\n";
    } else {
      echo "\"$e\" is not numeric\t***\n";
    }
  }

  $t = sprintf("%b,%d", 0b1010, 0b1010);
  var_dump($t);
  var_dump((string)INF);
  var_dump((string)-INF);
  var_dump((string)NAN);
  var_dump(-PHP_INT_MAX - 1);
  var_dump(-PHP_INT_MAX - 1 - 1);	// wraps to max positive

  var_dump(PHP_INT_MAX);
  var_dump(PHP_INT_MAX + 1);	// wraps to min negative

  var_dump(PHP_INT_MAX/2 + PHP_INT_MAX);	// converts to float
}

/* HH_FIXME[1002] call to main in strict*/
main();
