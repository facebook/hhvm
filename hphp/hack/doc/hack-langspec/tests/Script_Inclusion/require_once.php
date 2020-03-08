<?hh // strict

namespace NS_require_once;

//require_once('XXPositions.php');
require('Positions.php');
require_once('Point.php');
require_once('Circle.php');

function main(): void {
  var_dump(\NS_Positions\LEFT);
  var_dump(\NS_Positions\TOP);

  $p1 = new \NS_Point\Point(10.0,20.0);
  $p2 = new \NS_Point\Point(5.0, 6.0);
  var_dump($p1, $p2);

  $c1 = new \NS_Circle\Circle(9.0, 7.0, 2.4);
 var_dump($c1);

  print_r(get_required_files());
}

/* HH_FIXME[1002] call to main in strict*/
main();
