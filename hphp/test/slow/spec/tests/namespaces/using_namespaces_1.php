<?hh
use \Graphics\D2\Point;
//use \Graphics\D2\Point as Point;      // "as Point" is redundant
use \Graphics\D2\Circle as Circle;      // "as Circle" is redundant

use \Graphics\D2\Point as P;

use \Graphics\D2\Circle as C;
<<__EntryPoint>>
function entrypoint_using_namespaces_1(): void {

  /*
     +-------------------------------------------------------------+
     | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
     +-------------------------------------------------------------+
  */

  error_reporting(-1);

  include_once 'Point.inc';
  include_once 'Circle.inc';

  $p1 = new \Graphics\D2\Point(3, 5);     // works with/without use clause
  echo "\$p1 = $p1\n";
  $p1 = new Graphics\D2\Point(4, 6);      // works with/without use clause
  echo "\$p1 = $p1\n";
  $p1 = new Point(-3, 8);
  echo "\$p1 = $p1\n";

  $c1 = new \Graphics\D2\Circle(2, 4, 3.6);
  echo "\$c1 = $c1\n";
  $c1 = new Graphics\D2\Circle(3, 5, 4.7);
  echo "\$c1 = $c1\n";
  $c2 = new Circle(1, -2, 1.4);
  echo "\$c2 = $c2\n";
  $p1 = new P(-3, 8);
  echo "\$p1 = $p1\n";
  $c2 = new C(1, -2, 1.4);
  echo "\$c2 = $c2\n";

  echo "PHP_INT_MAX = " . PHP_INT_MAX . "\n";
  echo "PHP_INT_MAX = " . \PHP_INT_MAX . "\n";
}
