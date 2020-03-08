<?hh // strict

namespace NS_sleep_and_wakeup;

class Point {
  private static int $pointCount = 0;
  private static int $nextId = 1;

  private float $x;
  private float $y;
  private int $id;

  public static function getPointCount(): int {
    return self::$pointCount;
  }

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
    ++self::$pointCount;
    $this->id = self::$nextId++;

    echo "\nInside " . __METHOD__ . ", $this, point count = " . self::$pointCount . "\n\n";
  }

  public function move(float $x, float $y): void {
    $this->x = $x;
    $this->y = $y;
  }	

  public function translate(float $x, float $y): void {
    $this->x += $x;
    $this->y += $y;
  }

  public function __destruct() {
    --self::$pointCount;

    echo "\nInside " . __METHOD__ . ", $this, point count = " . self::$pointCount . "\n\n";
  }

  public function __toString(): string {
    return 'ID:' . $this->id . '(' . $this->x . ',' . $this->y . ')';
  }	

  public function __sleep(): array<string> {
    echo "\nInside " . __METHOD__ . ", $this, point count = " . self::$pointCount . "\n\n";
		
    return array('y', 'x');	// get serialized in array insertion order
  }

  public function __wakeup(): void {
    echo "\nInside " . __METHOD__ . ", $this, \$nextId, = " . self::$nextId . "\n\n";
		
    ++self::$pointCount;
    $this->id = self::$nextId++;
  }
}

class ColoredPoint extends Point {
  const int RED = 1;
  const int BLUE = 2;

  private int $color;

  public function __construct(float $x = 0.0, float $y = 0.0, int $color = ColoredPoint::RED) {
    parent::__construct($x, $y);
    $this->color = $color;

    echo "\nInside " . __METHOD__ . ", $this\n\n";
  }

  public function __toString(): string {
    return parent::__toString() . $this->color;
  }	

// while this method returns an array containing the names of the two inherited, private 
// properties and adds to that the one private property from the current class,
// serialize runs in the context of the type of the object given it. If that type is
// ColoredPoint, serialize doesn't know what to do when it comes across the names of the 
// inherited, private properties.

  public function __sleep(): array<string> {
    echo "\nInside " . __METHOD__ . ", $this\n\n";
		
    $a = parent::__sleep();
    var_dump($a);
    $a[] = 'color';
    var_dump($a);
    return $a;
  }
}

function main(): void {
  echo "---------------- create and destroy a Point to boost id -------------------\n";

  $a = new Point(1.0, 1.0);

  echo "---------------- create, serialize, and unserialize a Point -------------------\n";

  $p = new Point(-1.0, 0.0);
  echo "Point \$p = $p\n";

  $s = serialize($p);		// all instance properties get serialized
  var_dump($s);

  echo "------\n";

  $v = unserialize($s);	// without a __wakeup method, any instance property present 
						// in the string takes on its default value.
  var_dump($v);

  echo "---------------- Serialize and unserialize null -------------------\n";

  $s = serialize(null);	// simulate __sleep not having a return statement or returning nothing
  var_dump($s);

  $v = unserialize($s);
  var_dump($v);

  echo "---------------- Serialize ColoredPoint -------------------\n";

  $cp = new ColoredPoint(9.0, 8.0, ColoredPoint::BLUE);
  echo "ColoredPoint \$cp = $cp\n";

  $s = serialize($cp);
  var_dump($s);

  $v = unserialize($s);
  var_dump($v);

  echo "---------------- end -------------------\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
