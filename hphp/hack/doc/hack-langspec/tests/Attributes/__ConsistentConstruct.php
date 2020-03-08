<?hh // strict

namespace NS_ConsistentConstruct;

// --------------------------------------------------------
// test with constructors having no arguments, but override being variadic

<<__ConsistentConstruct>>
class Base {
  public function __construct() {
    echo "In " . __METHOD__ . "\n";
  }

  public static function make(): this {
    echo "making a new object\n";
    return new static();
  }
}

class Derived extends Base {
  public function __construct(...) {
    echo "In " . __METHOD__ . "\n";
    parent::__construct();
  }
}

// --------------------------------------------------------
// test with constructors having no 2 arguments, and override being variadic

<<__ConsistentConstruct>>
class C1 {
  public function __construct(int $p1, float $p2) {
    echo "In " . __METHOD__ . "\n";
  }

  public static function make(): this {
    echo "making a new object\n";
    return new static(123, 6.5);
  }
}

<<__ConsistentConstruct>>
class C2 extends C1 {
  public function __construct(int $p1, float $p2, ...) {
    echo "In " . __METHOD__ . "\n";
    parent::__construct($p1, $p2);
  }
}

/*
// --------------------------------------------------------
// use UNSAFE_Construct attribute

<<__ConsistentConstruct>>
class Base2 {
  public function __construct() {
    echo "In " . __METHOD__ . "\n";
  }

  public static function make(): this {
    echo "making a new object\n";
    return new static();
  }
}

class Derived2 extends Base2 {
  <<UNSAFE_Construct>>
  public function __construct(int $p1) {
    echo "In " . __METHOD__ . "\n";
    parent::__construct();
  }
}
*/
// --------------------------------------------------------

//<<__ConsistentConstruct, Attr2(3, true)>>	// hmmm! accepted
<<__ConsistentConstruct(3), Attr2(3, true)>>	// hmmm! accepted with a value
function f1(): void { echo "Inside " . __FUNCTION__ . "\n"; }

// --------------------------------------------------------

function main(): void {
echo "\n============== on constructor =====================\n\n";

  $v1 = Base::make();
  var_dump($v1);

  $v2 = Derived::make();
  var_dump($v2);

  $v1 = C1::make();
  var_dump($v1);

  $v2 = C2::make();
  var_dump($v2);

echo "\n============== top-level function f1 =====================\n\n";

  f1();
  $rf = new \ReflectionFunction('\NS_ConsistentConstruct\f1');
  $attr1 = $rf->getAttribute('__ConsistentConstruct');	// hmmm!
  var_dump($attr1);
  $attr2 = $rf->getAttribute('Attr2');
  var_dump($attr2);
}

/* HH_FIXME[1002] call to main in strict*/
main();
