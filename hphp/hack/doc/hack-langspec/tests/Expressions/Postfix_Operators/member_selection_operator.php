<?hh // strict

namespace NS_member_selection_operator;

class Point {
  private float $x;
  private float $y;

  public function getX(): float 	{ return $this->x; }
  public function setX(float $x): void	{ $this->x = $x;   }
  public function getY(): float		{ return $this->y; }
  public function setY(float $y): void	{ $this->y = $y;   }

  public function __construct(float $x = 0.0, float $y = 0.0) {
//  $name = 'x';
//  $this->$name = $x;	// Unlike PHP, Hack does not permit this
    $this->x = $x;
    $this->y = $y;
  }

  public function move(float $x, float $y): void {
    $this->x = $x;
    $this->y = $y;
  }	

  public function translate(float $x, float $y): void {
    $this->x += $x;
    $this->y += $y;
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }

  public int $piProp = 555;
  public static function psf(): int { return 123; }
  public static int $psProp = 999;
  const float MYPI = 3.14159;
}

class K {
  public L $prop;
  public function __construct(L $p1) {
    $this->prop = $p1;
  }
}

class L {
  public function f(): void { echo "Hello from f\n"; }
}

function main(): void {
  $p1 = new Point();
  echo "\$p1 is >$p1<\n";

  $p1->move(3.0, 9.0);
  echo "\$p1 is >$p1<\n";

//  $n = "move";
//  $p1->$n(-2.0, 4.0);	// Unlike PHP, Hack does not permit this
//  echo "\$p1 is >$p1<\n";

  var_dump($p1->piProp);	// okay to access instance property via instance
//  var_dump($p1->psf());	// Unlike PHP, Hack doesn't allow access to a static method via instance

  var_dump(Point::psf());	// okay to access static method via class
  var_dump(Point::$psProp);// okay to access static property via class, but leading $ needed!!
  var_dump(Point::MYPI);	// okay to access const via class

// use multiple ->s

  $k = new K(new L());
  $k->prop->f();
}

/* HH_FIXME[1002] call to main in strict*/
main();
