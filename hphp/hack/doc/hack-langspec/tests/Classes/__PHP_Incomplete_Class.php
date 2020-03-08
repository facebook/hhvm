<?hh // strict

namespace NS___PHP_Incomplete_Class;

class Point {
  private float $x;
  private float $y;

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;

    echo "\nInside " . __METHOD__ . ", $this\n\n";
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }	
}

function main(): void {
  echo "--- create a Point ---\n\n";

  $p = new Point(2.0, 5.0);
  echo "Point \$p = $p\n";

  echo "\n--- serialize that Point ---\n\n";

  $s = serialize($p);		// all instance properties get serialized
  var_dump($s);

  echo "\n--- unserialize that Point ---\n\n";

  $v = unserialize($s);
  var_dump($v);

  echo "\n--- fake string value ---\n\n";

  $s[32] = 'J';		// change class name, so an unserialize failure occurs
  var_dump($s);

  echo "\n--- unserialize that Point to non-existant class type ---\n\n";

  $v = unserialize($s);
  var_dump($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();
