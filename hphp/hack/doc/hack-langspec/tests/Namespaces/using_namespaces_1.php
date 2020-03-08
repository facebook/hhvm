<?hh // strict

namespace NS_using_namespaces1;

require_once('Point.php');
require_once('Circle.php');

use \Graphics\D2\Point;
//use \Graphics\D2\Point as Point;		// "as Point" is redundant

use \Graphics\D2\Circle as Circle;		// "as Circle" is redundant

function main(): void {
  $p1 = new \Graphics\D2\Point(3.0, 5.0);
  echo "\$p1 = $p1\n";
  $p1 = new Point(-3.0, 8.0);
  echo "\$p1 = $p1\n";

  $c1 = new \Graphics\D2\Circle(2.0, 4.0, 3.6);
  echo "\$c1 = $c1\n";
  $c2 = new Circle(1.0, -2.0, 1.4);
  echo "\$c2 = $c2\n";

//  use \Graphics\D2\Point as P;
//  $p1 = new P(-3.0, 8.0);
//  echo "\$p1 = $p1\n";

//  use \Graphics\D2\Circle as C;
//  $c2 = new C(1.0, -2.0, 1.4);
//  echo "\$c2 = $c2\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
