<?hh

class Point {
  private $x;
  private $y;

  public function getX() :mixed{
    return $this->x;
  }
  public function setX($x) :mixed{
    $this->x = $x;
  }
  public function getY() :mixed{
    return $this->y;
  }
  public function setY($y) :mixed{
    $this->y = $y;
  }

  public function __construct($x = 0, $y = 0) {
    $name = "x";
    $this->$name = $x; // member name as the value of a string is permitted
    //      $this->"x" = $x;    // string literal not allowed, however
    //      $this->x = $x;
    $this->y = $y;
  }

  public function move($x, $y) :mixed{
    $this->x = $x;
    $this->y = $y;
  }

  public function translate($x, $y) :mixed{
    $this->x += $x;
    $this->y += $y;
  }

  public function __toString() :mixed{
    return '('.$this->x.','.$this->y.')';
  }

  public $piProp = 555;
  public static function psf() :mixed{
    return 123;
  }
  public static $psProp = 999;
  const MYPI = 3.14159;
}

// use multiple ->s

class K {
  public $prop;
  public function __construct(L $p1) {
    $this->prop = $p1;
  }
}

class L {
  public function f() :mixed{
    echo "Hello from f\n";
  }
}
<<__EntryPoint>>
function entrypoint_member_selection_operator(): void {

  /*
     +-------------------------------------------------------------+
     | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
     +-------------------------------------------------------------+
  */

  error_reporting(-1);

  $p1 = new Point;
  echo "\$p1 is >$p1<\n";
  ///*
  $p1->move(3, 9);
  echo "\$p1 is >$p1<\n";

  $n = "move";
  $p1->$n(-2, 4);
  echo "\$p1 is >$p1<\n";

  $p1->color = "red"; // turned into $p1->__set("color", "red");
  var_dump($p1);

  $c = $p1->color; // turned into $c = $p1->__get("color");
  var_dump($c);
  //*/

  var_dump($p1->piProp); // okay to access instance property via instance
  //var_dump($p1->psf());     // not okay to access static method via instance
  //var_dump(($p1->psf)());   // doesn't parse
  //var_dump($p1->psf);       // so no surprise this won't work

  // Not okay. Strict Standards: Accessing static property
  // Point::$psProp as non static
  // Fatal: Undefined property: Point::$psProp
  try {
    var_dump($p1->psProp);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
  // Not okay. Fatal: Undefined property: Point::$MYPI
  try {
    var_dump($p1->MYPI);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }


  //var_dump(Point::piProp);// Fatal error: Undefined class constant 'piProp'
  var_dump(Point::psf()); // okay to access static method via class
  var_dump(
    Point::$psProp,
  ); // okay to access static property via class, but leading $ needed!!
  var_dump(Point::MYPI); // okay to access const via class

  $k = new K(new L);
  $k->prop->f();
}
