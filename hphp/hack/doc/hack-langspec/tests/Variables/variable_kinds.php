<?hh // strict

namespace NS_variable_kinds;

// global $min;			// disallowed
// global int $min;		// disallowed
// global int $min = -100;	// disallowed

// const int $gc = 10;		// disallowed

function localConst(bool $p): void {
//  const int $lc = 10;	// disallowed; declarations are not supported outside global scope

// define()'d names are not really constants in the linguistic sense, esp. as Hack can't see them directly

  echo "Inside " . __FUNCTION__ . "\n";

  define('COEFFICIENT_1', 2.345);
  echo "COEFFICIENT_1 = " . constant('COEFFICIENT_1') . "\n";

  if ($p) {
    echo "COEFFICIENT_1 = " . constant('COEFFICIENT_1') . "\n";

    define('FAILURE', true);
    echo "FAILURE = " . constant('FAILURE') . "\n";
  }
}

function doit(bool $p1): void {	// assigned the value true when called
  $count = 10;

  if ($p1) {
    $message = "Can't open master file.";
  }
}

function f(): void {
  $lv = 1;

  static $fs1 = 1;
//  static int $fs1 = 1;	// disallowed

//  var_dump($fs1);
//  $fs1 = true;
//  var_dump($fs1);
//  $fs1 = 123.45;
//  var_dump($fs1);

  static $fs2;
  var_dump($fs2);		// show default value is null

  echo "\$lv = $lv, \$fs1 = $fs1\n";
  ++$lv;
  ++$fs1;
  if (true) {
    static $fs3 = 99;
    echo "\$fs3 = $fs3\n";
    ++$fs3;
  }
}

function factorial(int $i): int {
  if ($i > 1) return $i * factorial($i - 1);
  else if ($i == 1) return $i;
  else return 0;
}

/*
// globals don't appear to be supported

function compute(bool $p): void {
  global $min, $max;
  global $average;
  $average = ($max + $min)/2;
  var_dump($min, $max, $average);

  if ($p) {
    global $result;
    $result = 3.456;	// initializes a global, creating it if necessary
    var_dump($result);
  }
}
*/

class Point {
  const int MAX_COUNT = 1000;

  private static int $pointCount = 0;

  public float $x;
  public float $y;

  public function __construct(float $newX = 0.0, float $newY = 0.0) {
    $this->x = $newX;
    $this->y = $newY;
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }	
}

function main(): void {
  echo "---------------- Local \"constants\" -------------------\n";

  localConst(true);
  echo "COEFFICIENT_1 = " . constant('COEFFICIENT_1') . "\n";
  echo "FAILURE = " . constant('FAILURE') . "\n";	// as it's visible here, it's not really a local!

  echo "---------------- Local variables -------------------\n";

  doit(true);

  echo "---------------- Function statics -------------------\n";

  for ($i = 1; $i <= 3; ++$i)
    f();

  echo "---------------- recursive function example -------------------\n";

  $result = factorial(10);
  echo "\$result = $result\n";

/*
  echo "---------------- create and use some globals -------------------\n";

  compute(true);
  echo "\$average = $average\n";
  echo "\$result = $result\n";
*/

  echo "------ instance/static properties & constants --------\n";

  echo "Point::MAX_COUNT: " . Point::MAX_COUNT . "\n";

  $p1 = new Point();
  echo "\$p1: $p1\n";

  $p2 = new Point(5.3, 6.8);
  echo "\$p2: $p2\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();


/*
//var_dump($GLOBALS);

  echo "---------------- Global Variables using \$GLOBALS -------------------\n";

$GLOBALS['done'] = false;
var_dump($done);

$GLOBALS['min'] = 10;
$GLOBALS['max'] = 100;
$GLOBALS['average'] = null;

global $min, $max;		// allowed, but serve no purpose

function compute2($p) {
  $GLOBALS['average'] = ($GLOBALS['max'] + $GLOBALS['min'])/2;

  if ($p) {
    $GLOBALS['result'] = 3.456;		// initializes a global, creating it if necessary
  }
}

compute2(true);
echo "\$average = $average\n";
echo "\$result = $result\n";

//var_dump($GLOBALS);
*/
