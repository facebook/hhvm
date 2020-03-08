<?hh // strict

require_once 'Point.php';
use NS_Point\Point;

function main(): void {
  error_reporting(-1);

  echo "\n==================== new Point ===================\n\n";

  $p1 = new Point();
  echo "\$p1 = $p1\n";
//  unset($p1);

  echo "\n==================== new Point() ===================\n\n";

  $p1 = new Point();
  var_dump($p1);
//  unset($p1);

  echo "\n==================== new Point(100) ===================\n\n";

  $p1 = new Point(100);		// let y default
  var_dump($p1);
//  unset($p1);

//  echo "\n==================== new 'Point'(-1, 1) ===================\n\n";
//
//  $cName = 'Point';
//  $p1 = new $cName(-1, 1);	// use string form
//  var_dump($p1);
//  unset($p1);

  echo "\n==================== new Point(20,30) ===================\n\n";

  $p1 = new Point(20,30.0);
  echo "\$p1 = $p1\n";

  echo "\n==================== \$p1->getX(), \$p1->getY() ===================\n\n";

  var_dump($p1->getX());
  var_dump($p1->getY());

  echo "\n==================== \$p1->setX(), \$p1->setY() ===================\n\n";

  $p1->setX(-3);
  $p1->setY(10);
  echo $p1 . "\n";					// implicit call to __toString()
  echo $p1->__toString() . "\n";		// explicit call to __toString()

  echo "\n==================== \$p1->move(-5, 7) ===================\n\n";

  $p1->move(-5, 7);
  echo $p1 . "\n";

  echo "\n==================== \$p1->translate(1, 1) ===================\n\n";

  $p1->translate(1, 1);
  echo $p1 . "\n";
//  unset($p1);

  echo "\n==================== create 3 Points then call getPointCount() ===================\n\n";

  $p1 = new Point();
  $p2 = new Point(-4,3);
  $p3 = new Point(20,30);

  echo "Point count = " . Point::getPointCount() . "\n";
//  $cName = 'Point';
//  echo "Point count = " . $cName::getPointCount() . "\n";
//  unset($p1, $p2, $p3);

  echo "\n==================== \$p1->iMethod(...) ===================\n\n";

  $p1 = new Point();
  $p1->iMethod(10, true, "abc");	// $p1->__call('iMethod', array(…))
//  unset($p1);

  echo "\n==================== Point::sMethod(NULL, 1.234) ===================\n\n";

  Point::sMethod(null, 1.234);	// Point::__callStatic('sMethod', array(…))

  echo "\n==================== \$p1 = new Point(3.123, 6.2) ===================\n\n";

  $p1 = new Point(3.123, 6.2);
  $p2 = clone $p1;
  echo "\$p1 = $p1\n";
  echo "\$p2 = $p2\n";
//  unset($p1, $p2);

//  echo "\n==================== set/get dynamic property ===================\n\n";

//
//  $p1 = new Point(2, 1);
//  $p1->dummy = "abc";            // set dynamic property
//  var_dump($p1);
//  $v = $p1->dummy;             // get dynamic property
//  echo "dummy = $v\n";

//  echo "\n==================== isset ===================\n\n";

//
//  var_dump(isset($p1->dummy));     // test if dummy exists and is accessible, or is dynamic

  echo "\n==================== unset ===================\n\n";

//  unset($p1->dummy);
//  unset($p1);

//  echo "\n==================== __invoke() ===================\n\n";

//
//  $p1 = new Point(2.3, 4.5);
//  $r = $p1(123);		// becomes $r = $p1->__invoke(123);	
//  echo "\$r = $r\n";
//  unset($p1);

  echo "\n==================== __set_state/var_export ===================\n\n";

  $p1 = new Point(3, 5);
  $v = var_export($p1, true);	// returns string representation of $p1
  var_dump($v);
//  unset($p1);

  echo "\n==================== __sleep/__wakeup ===================\n\n";

  $p1 = new Point(-1, 0);
  $s = serialize($p1);		// serialize Point(-1,0)
//  echo "\$s = >$s<\n";
  var_dump($s);
  $v = unserialize($s);		// unserialize Point(-1,0)
//  echo "\$v = >$v<\n";
  var_dump($v);
//  unset($p1);

  echo "\n==================== end ===================\n\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
