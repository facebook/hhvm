<?hh // strict

namespace NS_while;

function main(): void {
  $i = 1;
  while ($i <= 10) {
    echo "$i\t".($i * $i)."\n";	// output a table of squares
    ++$i;
  }

  $count = 0;
  $done = false;
  while (true) {
    if (++$count == 5)
      $done = true;
    echo $count."\n";
    // ...
    if ($done)
      break;	// break out of the while loop
    // ...
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();
