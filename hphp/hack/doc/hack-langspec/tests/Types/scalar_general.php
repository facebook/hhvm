<?hh // strict

namespace NS_scalar_general;

class C {}

function main(): void {
  $infile = fopen("Testfile.txt", 'r');

  echo "========== is_numeric tests ==========\n\n";

  echo "is_numeric(true): " . (is_numeric(true) ? "True" : "False") . "\n";
  echo "is_numeric(false): " . (is_numeric(false) ? "True" : "False") . "\n";
  echo "is_numeric(10): " . (is_numeric(10) ? "True" : "False") . "\n";
  echo "is_numeric(12.34): " . (is_numeric(12.34) ? "True" : "False") . "\n";
  echo "is_numeric(\"\"): " . (is_numeric("") ? "True" : "False") . "\n";
  echo "is_numeric(\"123\"): " . (is_numeric("123") ? "True" : "False") . "\n";
  echo "is_numeric(\"123abc\"): " . (is_numeric("123abc") ? "True" : "False") . "\n";
  echo "is_numeric(\"abc\"): " . (is_numeric("abc") ? "True" : "False") . "\n";
  echo "is_numeric(null): " . (is_numeric(null) ? "True" : "False") . "\n";
  echo "is_numeric(array(10,20)): " . (is_numeric(array(10,20)) ? "True" : "False") . "\n";
  echo "is_numeric(new C()): " . (is_numeric(new C()) ? "True" : "False") . "\n";
  echo "is_numeric(resource): " . (is_numeric($infile) ? "True" : "False") . "\n";

  echo "\n========== is_numeric tests ==========\n\n";

  echo "is_scalar(true): " . (is_scalar(true) ? "True" : "False") . "\n";
  echo "is_scalar(10): " . (is_scalar(10) ? "True" : "False") . "\n";
  echo "is_scalar(12.34): " . (is_scalar(12.34) ? "True" : "False") . "\n";
  echo "is_scalar(\"123\"): " . (is_scalar("123") ? "True" : "False") . "\n";
  echo "is_scalar(null): " . (is_scalar(null) ? "True" : "False") . "\n";
  echo "is_scalar(array(10,20)): " . (is_scalar(array(10,20)) ? "True" : "False") . "\n";
  echo "is_scalar(new C()): " . (is_scalar(new C()) ? "True" : "False") . "\n";
  echo "is_scalar(resource): " . (is_scalar($infile) ? "True" : "False") . "\n";

  echo "\n ========== is_numeric tests ==========\n\n";

  echo "is_null(true): " . (is_null(true) ? "True" : "False") . "\n";
  echo "is_null(10): " . (is_null(10) ? "True" : "False") . "\n";
  echo "is_null(12.34): " . (is_null(12.34) ? "True" : "False") . "\n";
  echo "is_null(\"123\"): " . (is_null("123") ? "True" : "False") . "\n";
  echo "is_null(null): " . (is_null(null) ? "True" : "False") . "\n";
  echo "is_null(array(10,20)): " . (is_null(array(10,20)) ? "True" : "False") . "\n";
  echo "is_null(new C()): " . (is_null(new C()) ? "True" : "False") . "\n";
  echo "is_null(resource): " . (is_null($infile) ? "True" : "False") . "\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
