<?hh // strict

namespace NS_break;

function main(): void {
  for ($i = 1; $i <= 5; ++$i) {
    echo "\$i = $i\n";
    if ($i == 3)
      break;
  }

  for ($i = 10; $i <= 40; $i +=10) {
    echo "\n\$i = $i: ";
    switch($i) {
    case 10: echo "ten"; break;
    case 20: echo "twenty"; break;
    case 30: echo "thirty"; break;
    }
    echo "\nJust beyond the switch";
  }
  echo "\n----------\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
