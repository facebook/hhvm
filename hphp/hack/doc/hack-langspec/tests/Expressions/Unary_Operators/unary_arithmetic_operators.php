<?hh // strict

namespace NS_unary_arithmetic_operators;

function DoItInt(int $a): void {
  echo "--- start DoItInt -------------------------\n\n";
  echo "     original: "; var_dump($a);
  $b = +$a;
  $c = -$a;
  $d = !$a;

  $f = ~$a;
  echo "after unary ~: "; var_dump($f);
  printf(" before Hex: %08X\n", $a);
  printf(" after  Hex: %08X\n", $f);

  echo " before (int): ".(int)$a;
  printf("; before (int) Hex: %08X\n", $a);

  echo "\n--- end DoItInt -------------------------\n\n";
}

function DoItFloat(float $a): void {
  echo "--- start DoItFloat -------------------------\n\n";
  echo "     original: "; var_dump($a);
  $b = +$a;
  $c = -$a;
  $d = !$a;

  echo "\n--- end DoItFloat -------------------------\n\n";
}

function DoItBool(bool $a): void {
  echo "--- start DoItBool -------------------------\n\n";
  echo "     original: "; var_dump($a);
//  $b = +$a;		// a bool is not numeric, so can't be used here
//  $c = -$a;		// a bool is not numeric, so can't be used here
  $d = !$a;
  var_dump($d);
//  $f = ~$a;		// a bool is not numeric, so can't be used here

  echo "\n--- end DoItBool -------------------------\n\n";
}

function DoItString(string $a): void {
  echo "--- start DoItString -------------------------\n\n";
  echo "     original: "; var_dump($a);
//  $b = +$a;		// a string is not numeric, so can't be used here
//  $c = -$a;		// a string is not numeric, so can't be used here
  $d = !$a;
  var_dump($d);
//  $f = ~$a;		// a string is not numeric, so can't be used here

  echo "\n--- end DoItString -------------------------\n\n";
}

function main(): void {
///*
// arithmetic operands

  DoItInt(0);
  DoItInt(5);
  DoItInt(-10);
//  DoItInt(PHP_INT_MAX);		// Hack restriction; not yet supporting top-level constants
//  DoItInt(-PHP_INT_MAX - 1);	// Hack restriction; not yet supporting top-level constants

  DoItFloat(0.0);
  DoItFloat(0.0000001e-100);
  DoItFloat(12.7345);
  DoItFloat(-9.34E26);
//  DoItFloat(PHP_INT_MAX + 10);	// Hack restriction; not yet supporting top-level constants
  DoItFloat(1234567E50);
  DoItFloat(1234567E100);
  DoItFloat(INF);
  DoItFloat(-INF);
  DoItFloat(NAN);
  DoItFloat(-NAN);
//*/

///*
// null operand

//  $b = +null;		// null is not numeric, so can't be used here
//  $c = -null;		// null is not numeric, so can't be used here
  $d = !null;
  var_dump($d);
//  $f = ~null;		// null is not numeric, so can't be used here
//*/

//*
// Boolean operands

  DoItBool(true);
  DoItBool(false);
//*/

///*
// string operands

  DoItString("0");
  DoItString("-43");
  DoItString("123");
  DoItString("0.0");
  DoItString("-25.5e-10");
  DoItString("");
  DoItString("ABC");
//*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
