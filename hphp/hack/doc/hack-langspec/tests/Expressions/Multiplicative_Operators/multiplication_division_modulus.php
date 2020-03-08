<?hh // strict

namespace NS_multiplication_division_modulus;

function doit(num $p1_num): void {
  $oper = array(100, -3.4, $p1_num);
/*
  foreach ($oper as $e1) {
    foreach ($oper as $e2) {
      echo ">$e1< * >$e2<, result: "; var_dump($e1 * $e2);
    }
    echo "-------------------------------------\n";
  }
*/

/*
  foreach ($oper as $e1) {
    foreach ($oper as $e2) {
      if (($e2) == 0) continue;	// skip divide-by-zeros

      echo ">$e1< / >$e2<, result: "; var_dump($e1 / $e2);
    }
    echo "-------------------------------------\n";
  }
*/

///*
  $oper = array(100, -3);
  foreach ($oper as $e1) {
    foreach ($oper as $e2) {
      if (((int)$e2) == 0) continue;	// skip divide-by-zeros

        echo ">$e1< % >$e2<, result: "; var_dump($e1 % $e2);
      }
      echo "-------------------------------------\n";
    }

//  var_dump(10 % 3.0);		// % requires int operands
//  var_dump(10.0 % 3);		// % requires int operands
//  var_dump(10 % $p1_num);		// % requires int operands
//  var_dump($p1_num % 3);		// % requires int operands

  if (is_int($p1_num)) {
    echo ">$p1_num< % >3<, result: "; var_dump($p1_num % 3); // OK; num contains an int
  }
//*/
}

function main(): void {
  doit(11);
}

/* HH_FIXME[1002] call to main in strict*/
main();
