<?hh // strict

namespace NS_Point;

class Point {
  private static int $nextId = 1;
  private array<string,mixed> $dynamicProperties = array();
  private static int $pointCount = 0;
  private int $id;

  private float $x;		// Cartesian x-coordinate
  private float $y;		// Cartesian y-coordinate

  public function getX(): float  { return $this->x; }
  public function setX(num $x):void 	{ $this->x = (float)$x;   }
  public function getY(): float  { return $this->y; }
  public function setY(num $y): void   { $this->y = (float)$y;   }

  public static function getPointCount(): int {
    return self::$pointCount;
  }

  public function __construct(num $x = 0, num $y = 0) {
    $this->x = (float)$x;
    $this->y = (float)$y;
    ++self::$pointCount;
    $this->id = self::$nextId++;	// assign the next available id
    echo "Inside " . __METHOD__ . ", point count = " . self::$pointCount . "\n";
  }

  public function __destruct() {
    --self::$pointCount;
    echo "Inside " . __METHOD__ . ", point count = " . self::$pointCount . "\n";
  }

  public function move(num $x, num $y): void {
    $this->x = (float)$x;
    $this->y = (float)$y;
  }	

  public function translate(num $x, num $y): void {
    $this->x += $x;
    $this->y += $y;
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }	

  public function __clone(): void {
    ++self::$pointCount;
    echo "Inside " . __METHOD__ . ", point count = " . self::$pointCount . "\n";
  }

  public function __call(string $name, array<mixed> $arguments) : mixed	{
    echo "Inside " . __METHOD__ . ", \$name = $name\n";
    var_dump($arguments); echo "\n";
  }

  public static function __callStatic(string $name, array<mixed> $arguments) : mixed {
    echo "Inside " . __METHOD__ . ", \$name = $name\n";
    var_dump($arguments); echo "\n";
  }

  public function __get(string $name): mixed {
    echo "Inside " . __METHOD__ . ", \$name = $name\n";

    if (array_key_exists($name, $this->dynamicProperties)) {
      return $this->dynamicProperties[$name];
    }

    // no-such-property error handling goes here
    return null;
  }

  public function __set(string $name, mixed $value): void {
    echo "Inside " . __METHOD__ . ", \$name = $name\n";
    var_dump($value); echo "\n";

    $this->dynamicProperties[$name] = $value;
  }

  public function __isset(string $name): bool {
    echo "Inside " . __METHOD__ . ", \$name = $name\n";

    return true; //isset($this->dynamicProperties[$name]);
  }

  public function __unset(string $name): void {
    echo "Inside " . __METHOD__ . ", \$name = $name\n";

//  unset($this->dynamicProperties[$name]);
  }

  public function __invoke(...): mixed {
    echo "Inside " . __METHOD__ . "\n";
//  var_dump($p); echo "\n";

    return true;
  }

  static public function __set_state(array<string,mixed> $properties): Point {
    echo "Inside " . __METHOD__ . "\n";
    var_dump($properties); echo "\n";

    $p = new Point();
    $p->x = (float)$properties['x'];
    $p->y = (float)$properties['y'];
    return $p;
  }

  public function __sleep(): array<string> {
    echo "Inside " . __METHOD__ . "\n";

    return array('y', 'x');	// serialize only $y and $x, in that order
  }

  public function __wakeup(): void {
    echo "Inside " . __METHOD__ . "\n";

    $this->id = self::$nextId++;	// assign a new id
  }
}
