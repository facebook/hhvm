<?hh // strict

namespace NS_conditional;

function f(int $a): int { echo "inside f(int $a)\n"; return 0;}

function main(): void {
/*
  echo "======= check for even integer values by inspecting the low-order bit =======\n";

  for ($i = -5; $i <= 5; ++$i) {
    echo "$i is ".((($i & 1) == 1) ? "odd\n" : "even\n");
  }

*/
/*
  echo "======= some simple examples =======\n";

  $a = 10 ? 100 : "Hello";
  var_dump($a);
  $a = 0 ? 100 : "Hello";
  var_dump($a);

*/
/*
  echo "======= omit 2nd operand =======\n";

  $a = 10 ? : "Hello";
  var_dump($a);
  $a = 0 ? : "Hello";
  var_dump($a);

*/
/*
  echo "======= put a side effect in the 1st operand =======\n";

  $i = 5;
  $a = $i++ ? : "red";
  var_dump($a);
  $i = 5;
  $a = ++$i ? : "red";
  var_dump($a);

*/
/*
  echo "======= sequence point =======\n";

  $i = 5;
  $i++ ? f($i) : f(++$i);
  $i = 0;
  $i++ ? f($i) : f(++$i);

*/
/*
  echo "======= check associativity -- NOT the same as C/C++ =======\n";

  $a = true ? -1 : 1 ? 10 : 20;
  $a = (true ? -1 : 1) ? 10 : 20;
  var_dump($a);
  $a = true ? -1 : (1 ? 10 : 20);
*/
/*
  echo "======= Test all kinds of scalar values to see which are ints or can be implicitly converted =======\n";

  $scalarValueList = array(10, -100, 0, 1.234, 0.0, true, false, null, "123", 'xx', "");
  foreach ($scalarValueList as $v) {
    echo "\$v = $v, ";
    $a = $v ? 100 : "Hello";
    var_dump($a);
  }

*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
