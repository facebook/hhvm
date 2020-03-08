<?hh // strict

namespace NS_property_initializer;

class Point {
  public float $x = -10.0;	// gets applied before constructor runs
  public float $y = 10.0;		// gets applied before constructor runs

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }	
}

function main(): void {
  $p = new Point();
  echo $p . "\n";

  $p = new Point(100.0);
  echo $p . "\n";

  $p = new Point(1000.0, 2000.0);
  echo $p . "\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
