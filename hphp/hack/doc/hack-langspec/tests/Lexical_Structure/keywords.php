<?hh // strict

namespace NS_keywords;

function main(): void {
  $colors = array("red", "white", "blue");

  foreach ($colors as $a) {
    echo $a.' ';
  }
  echo "\n";

/* keywords must be lower-case

// try various case combinations of keywords

  fOREacH ($colors As $a) {
    eChO $a.' ';
  }
  ECHO "\n";
*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
