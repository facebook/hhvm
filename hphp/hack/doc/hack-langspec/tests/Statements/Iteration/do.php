<?hh // strict

namespace NS_do;

function main(): void {
  $i = 1;
  do {
    echo "$i\t".($i * $i)."\n";	// output a table of squares
    ++$i;
  }
  while ($i <= 10);
}

/* HH_FIXME[1002] call to main in strict*/
main();
