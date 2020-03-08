<?hh // strict

namespace NS_Memoize;

// --------------------------------------------------------

class Item {
  <<__Memoize>>
  public static function getNameFromProductCode(int $productCode): string {
    /* ... */
    return Item::getNameFromStorage($productCode);
  }

  private static function getNameFromStorage(int $productCode): string {
    $names = array('??', 'door', 'window', 'cabinet');
    echo "Inside " . __FUNCTION__ . "\n";
    return $names[$productCode];
  }
}

// --------------------------------------------------------

trait T {
  <<__Memoize>>
  public function getNameFromProductCode(int $productCode): string {
    /* ... */
    return $this->getNameFromStorage($productCode);
  }

  private function getNameFromStorage(int $productCode): string {
    $names = array('??', 'door', 'window', 'cabinet');
    echo "Inside " . __FUNCTION__ . "\n";
    return $names[$productCode];
  }
}

class C1 {
  use T;
}

// --------------------------------------------------------

// hhvm - <<__Memoize>> cannot be used in interfaces

interface I {
//  <<__Memoize>>
  public function getNameFromProductCode(int $productCode): string;
}

class C2 implements I {
  public function getNameFromProductCode(int $productCode): string {
    /* ... */
    return $this->getNameFromStorage($productCode);
  }

  private function getNameFromStorage(int $productCode): string {
    $names = array('??', 'door', 'window', 'cabinet');
    echo "Inside " . __FUNCTION__ . "\n";
    return $names[$productCode];
  }
}

// --------------------------------------------------------

<<__Memoize>>
function getNameFromProductCode(int $productCode): string {
  /* ... */
  return getNameFromStorage($productCode);
}

function getNameFromStorage(int $productCode): string {
  $names = array('??', 'door', 'window', 'cabinet');
  echo "Inside " . __FUNCTION__ . "\n";
  return $names[$productCode];
}

// --------------------------------------------------------

//<<__Memoize, Attr2(3, true)>>	// hmmm! accepted even though function has a void return type
<<__Memoize(3), Attr2(3, true)>>	// hmmm! accepted with a value
function f1(): void { echo "Inside " . __FUNCTION__ . "\n"; }

// --------------------------------------------------------

enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}

type Point = shape('x' => int, 'y' => int);

class C3 implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "xxxx";
  }
}

class C4 {} // does NOT implement IMemoizeParam

// checkout the permitted parameter types on the function

<<__Memoize>>
function getNameFromProductCode2(
	bool $p1
	, int $productCode
	, float $p3
	, string $p4
	, ?int $p5
//	, resource $p6		// invalid type
	, array<int> $p7
	, array<string, int> $p8
	, ControlStatus $p9	// permitted, but no in docs list
	, (int, float) $p10	// permitted, but no in docs list
	, Point $p11		// permitted, but no in docs list
	, Vector<int> $p12	// permitted, presumably this type implements IMemoizeParam
	, C3 $p13
//	, C4 $p14		// invalid type
): string {
  /* ... */
  return getNameFromStorage2($productCode);
}

function getNameFromStorage2(int $productCode): string {
  $names = array('??', 'door', 'window', 'cabinet');
  echo "Inside " . __FUNCTION__ . "\n";
  return $names[$productCode];
}

// --------------------------------------------------------

// try a variadic function

<<__Memoize>>
function getNameFromProductCode3(int $productCode, ...): string {
  /* ... */
  return getNameFromStorage3($productCode);
}

function getNameFromStorage3(int $productCode): string {
  $names = array('??', 'door', 'window', 'cabinet');
  echo "Inside " . __FUNCTION__ . "\n";
  return $names[$productCode];
}

// --------------------------------------------------------

function main(): void {
  echo "\n============== in class =====================\n\n";

  var_dump(Item::getNameFromProductCode(3));
  var_dump(Item::getNameFromProductCode(1));
  var_dump(Item::getNameFromProductCode(2));
  var_dump(Item::getNameFromProductCode(1));
  var_dump(Item::getNameFromProductCode(2));
  var_dump(Item::getNameFromProductCode(3));

  echo "\n============== in trait =====================\n\n";

  $c1 = new C1();
  var_dump($c1->getNameFromProductCode(3));
  var_dump($c1->getNameFromProductCode(1));
  var_dump($c1->getNameFromProductCode(2));
  var_dump($c1->getNameFromProductCode(1));
  var_dump($c1->getNameFromProductCode(2));
  var_dump($c1->getNameFromProductCode(3));

  echo "\n============== in interface =====================\n\n";

  $c2 = new C2();
  var_dump($c2->getNameFromProductCode(3));
  var_dump($c2->getNameFromProductCode(1));
  var_dump($c2->getNameFromProductCode(2));
  var_dump($c2->getNameFromProductCode(1));
  var_dump($c2->getNameFromProductCode(2));
  var_dump($c2->getNameFromProductCode(3));

  echo "\n============== top-level function getNameFromProductCode =====================\n\n";

  var_dump(getNameFromProductCode(3));
  var_dump(getNameFromProductCode(1));
  var_dump(getNameFromProductCode(2));
  var_dump(getNameFromProductCode(1));
  var_dump(getNameFromProductCode(2));
  var_dump(getNameFromProductCode(3));

  echo "\n============== top-level function getNameFromProductCode2 =====================\n\n";

  var_dump(getNameFromProductCode2(true, 3, 1.2, "z", null
//		, STDERR			// invalid type
		, array(10,20)
		, array('q' => 10, 'a' => 12)
		, ControlStatus::Stopped	// permitted, but no in docs list
		, tuple(10, 1.2)		// permitted, but no in docs list
		, shape('x' => 1, 'y' => 3)	// permitted, but no in docs list
		, Vector {10,20,30}		// permitted, but no in docs list
		, new C3()			// permitted, presumably this type implements IMemoizeParam
//		, new C4()			// invalid type
  ));
  var_dump(getNameFromProductCode2(true, 3, 1.2, "z", null
//		, STDERR			// invalid type
		, array(10,20)
		, array('q' => 10, 'a' => 12)
		, ControlStatus::Stopped	// permitted, but no in docs list
		, tuple(10, 1.2)		// permitted, but no in docs list
		, shape('x' => 1, 'y' => 3)	// permitted, but no in docs list
		, Vector {10,20,30}		// permitted, but no in docs list
		, new C3()			// permitted, presumably this type implements IMemoizeParam
//		, new C4()			// invalid type
  ));

  echo "\n============== top-level function getNameFromProductCode3 =====================\n\n";

  var_dump(getNameFromProductCode3(3));
  var_dump(getNameFromProductCode3(1));
  var_dump(getNameFromProductCode3(2));
  var_dump(getNameFromProductCode3(1));
  var_dump(getNameFromProductCode3(2));
  var_dump(getNameFromProductCode3(3));

  echo "\n============== top-level function f1 =====================\n\n";

  f1();
  $rf = new \ReflectionFunction('\NS_Memoize\f1');
  $attr1 = $rf->getAttribute('__Memoize');	// hmmm!
  var_dump($attr1);
  $attr2 = $rf->getAttribute('Attr2');
  var_dump($attr2);
}

/* HH_FIXME[1002] call to main in strict*/
main();
