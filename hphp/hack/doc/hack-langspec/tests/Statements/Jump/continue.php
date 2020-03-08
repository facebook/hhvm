<?hh // strict

namespace NS_continue;

function main(): void {
  for ($i = 1; $i <= 5; ++$i) {
    if (($i % 2) == 0)
      continue;
    echo "$i is odd\n";
  }
/*
// continue N/break N not supported

  for ($i = 1; $i <= 5; ++$i) {
    $j = 20;
    while ($j > 0) {
      if ((($j * $i) % 2) == 0) {
        $j -= 3;
        continue 1;
      }
      echo ($j * $i)." is odd\n";
      $j -= 5;
    }
    echo "In for loop\n";
  }
*/

// In PHP, 'continue;' inside a switch statement is equivalent to 'break;'. Hack does not 
// support this; use 'break' if that is what you meant.

  for ($i = 10; $i <= 40; $i +=10) {
    echo "\n\$i = $i: ";
    switch($i) {
    case 10: echo "ten"; break;
//    case 20: echo "twenty"; continue;
    case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
  }
  echo "\n----------\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
