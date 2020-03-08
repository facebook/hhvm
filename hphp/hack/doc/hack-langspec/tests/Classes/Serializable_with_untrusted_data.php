// NOT YET IMPLEMENTED IN CHECKER OR ENGINE <?hh // strict

namespace NS_Serializable_with_untrusted_data;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014-2016, 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

class Point implements \Serializable {
  private static int $nextId = 1;

  private int $x;
  private int $y;
  private int $id;

  public function __construct(int $x = 0, int $y = 0) {
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

  public function __construct(int $x = 0, int $y = 0, int $color = ColoredPoint::RED) {
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

  $p = new Point(2, 5);
  echo "Point \$p = $p\n";

  $s = serialize($p);
  var_dump($s);

  echo "------\n";

//  $v = unserialize($s, ["allowed_classes" => true]);	// OK
  $v = unserialize($s, ["allowed_classes" => false]);		// converts Point to __PHP_Incomplete_Class
  var_dump($v);

  echo "---------------- Serialize ColoredPoint -------------------\n";

  $cp = new ColoredPoint(9, 8, ColoredPoint::BLUE);
  echo "ColoredPoint \$cp = $cp\n";

  $s = serialize($cp);
  var_dump($s);

//  $v = unserialize($s, ["allowed_classes" => true]);	// OK
  $v = unserialize($s, ["allowed_classes" => false]);		// converts ColoredPoint to __PHP_Incomplete_Class
  var_dump($v);

  echo "---------------- end -------------------\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
