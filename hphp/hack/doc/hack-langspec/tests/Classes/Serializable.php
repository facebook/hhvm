<?hh // strict

namespace NS_Serializable;

class Point implements \Serializable {
  private static int $nextId = 1;

  private float $x;
  private float $y;
  private int $id;

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
    $this->id = self::$nextId++;

    echo "\nInside " . __METHOD__ . ", $this\n\n";
  }

  public function __toString(): string {
    return 'ID:' . $this->id . '(' . $this->x . ',' . $this->y . ')';
  }	

  public function serialize(): string {
    echo "\nInside " . __METHOD__ . ", $this\n\n";
		
    return serialize(array('y' => $this->y, 'x' => $this->x));
  }
    
  public function unserialize(string $data): void {
    $data = unserialize($data);
    $this->x = $data['x'];
    $this->y = $data['y'];
    $this->id = self::$nextId++;

    echo "\nInside " . __METHOD__ . ", $this\n\n";
  }
}

class ColoredPoint extends Point implements \Serializable {
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

  public function serialize(): string {
    echo "\nInside " . __METHOD__ . ", $this\n\n";
		
    return serialize(array(
      'color' => $this->color,
      'baseData' => parent::serialize()
    ));
  }
    
  public function unserialize(string $data): void {
    $data = unserialize($data);
    $this->color = $data['color'];
    parent::unserialize($data['baseData']);

    echo "\nInside " . __METHOD__ . ", $this\n\n";
  }
}

function main(): void {
  echo "---------------- create, serialize, and unserialize a Point -------------------\n";

  $p = new Point(2.0, 5.0);
  echo "Point \$p = $p\n";

  $s = serialize($p);
  var_dump($s);

  echo "------\n";

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
