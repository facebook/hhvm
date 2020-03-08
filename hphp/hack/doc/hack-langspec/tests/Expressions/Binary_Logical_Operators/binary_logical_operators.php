<?hh // strict

namespace NS_binary_logical_operators;

function f(int $a): int { echo "inside f(int $a)\n"; return 10;}
function g(int $a): int { echo "inside g(int $a)\n"; return 0;}

function main(): void {
  $month = 6;
  if ($month > 1 && $month <= 12)
    echo "\$month $month is in-bounds\n";
  else
    echo "\$month $month is out-of-bounds\n";

  $month = 14;
  if ($month > 1 && $month <= 12)
    echo "\$month $month is in-bounds\n";
  else
    echo "\$month $month is out-of-bounds\n";

  $month = 6;
  if ($month < 1 || $month > 12)
    echo "\$month $month is out-of-bounds\n";
  else
    echo "\$month $month is in-bounds\n";

  $month = 14;
  if ($month < 1 || $month > 12)
    echo "\$month $month is out-of-bounds\n";
  else
    echo "\$month $month is in-bounds\n";

// sequence point

  $i = 5;
  $v = (f($i++) && g($i));
  var_dump($v);
  $i = 0;
  $v = (g($i++) || f($i));
  var_dump($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();
