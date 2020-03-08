<?hh // strict

namespace NS_cloning;

require_once 'Point2.php';
use NS_Point2\Point2;

class C {
  private int $m;
  public function __construct(int $p1) {
    $this->m = $p1;
  }

///*
  public function __clone(): void {
    echo "Inside " . __METHOD__ . "\n";
  }
//*/
}

class Employee {
  private string $name;

  public function __construct(string $name) {
    $this->name = $name;
  }

  public function __clone(): void {
    echo "Inside " . __METHOD__ . "\n";

    // make a copy of Employee object
  }
}

class Manager extends Employee {
  private int $level;

  public function __construct(string $name, int $level) {
    parent::__construct($name);
    $this->level = $level;
  }

  public function __clone(): void {
    echo "Inside " . __METHOD__ . "\n";

    parent::__clone();

    // make a copy of Manager object
  }
}

function main(): void {
  $obj1 = new C(10);
  var_dump($obj1);

  $obj2 = clone $obj1;	// default action is to make a shallow copy
  var_dump($obj2);

  echo "================= Use cloning in Point class =================\n";

  echo "Point count = " . Point2::getPointCount() . "\n";
  $p1 = new Point2(-3.5, 1.4);
  var_dump($p1);
  echo "Point count = " . Point2::getPointCount() . "\n";
  $p2 = clone $p1;
  var_dump($p2);
  echo "Point count = " . Point2::getPointCount() . "\n";

  var_dump($p3 = clone $p1);
  echo "Point count = " . Point2::getPointCount() . "\n";

  var_dump($p4 = clone $p1);
  echo "Point count = " . Point2::getPointCount() . "\n";

  echo "================= use chained cloning in a class heirarchy =================\n";

  $obj3 = new Manager("Smith", 23);
  var_dump($obj3);

  $obj4 = clone $obj3;
  var_dump($obj4);
}

/* HH_FIXME[1002] call to main in strict*/
main();
