<?hh // strict

namespace NS_shapes;

require_once 'shapes_rf.php';

function f1(\NS_shapes_rf\st2a $p1): void {
  echo "----- dumping 'x' => int -----\n\n";

  var_dump($p1);
}

function main(): void {
/*
// yes, array string keys can not only be "...", can also contain subtitution stuff
// testing, as double-quoted strings keys are rejected in Shape definitions.

  $q = 'QQQ';
  $a = array('x' => 10, "y" => 20, "x\n$q\tx" => 30,
<<<ID
XXX
ID
 => 40, 
<<<'ID'
YYY
ID
 => 50);
  var_dump($a);
*/

  echo "----------- Play ------------\n\n";

//  $key = 'x';
//  f1(shape($key => 10));	// gags: The field '' is missing
				// like the shape definition, key must be a string literal

//  var_dump(\NS_shapes_rf\st4b_test2());

  echo "----------- Create some shape values ------------\n\n";

  $point1 = shape('x' => -3, 'y' => 6);
  echo "\$point1 is " . $point1['x'] . "\n";
//  echo "\$point1 instanceof Point is " . 
//    (($point1 instanceof \NS_shapes_rf\Point) ? "True" : "False") . "\n";	// False

/*
// a key must be a string literal; apparently can't be a class constant

// ???	echo "\$point1 is " . $point1[\NS_shapes_rf\C::KEY1] . "\n";
	// The field  is undefined *** WHY IS THIS?
  $key = 'x';
  echo "\$point1 is " . $point1[$key] . "\n";	// The field  is undefined

  echo "\$point1 is " . $point1['z'] . "\n";	// The field z is undefined
*/

// Fatal error: syntax error, unexpected '[', expecting ',' or ';'

//  echo "shape(...)['y'] is " . shape('x' => -3, 'y' => 6)['y'] . "\n";

  echo "shape(...)['y'] is " . (shape('x' => -3, 'y' => 6))['y'] . "\n";
	// works with parens, but shouldn't need them

  $str = \NS_shapes_rf\Point_toString($point1);
  echo "\$point1 is $str\n";
//  echo "\$point1 instanceof Point is " . 
//    (($point1 instanceof \NS_shapes_rf\Point) ? "True" : "False") . "\n";	// False

// The following function-call argument is incompatible with PointNT while alias is created using 'newtype'.
// However, it's okay when 'type' used

//  $str = \NS_shapes_rf\PointNT_toString($point1);
//  echo "\$point1 is $str\n";

  $str = \NS_shapes_rf\PointNT_toString(\NS_shapes_rf\PointNT_getOrigin());
  echo "Origin is $str\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
