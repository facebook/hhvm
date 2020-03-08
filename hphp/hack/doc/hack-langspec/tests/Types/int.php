<?hh // strict

namespace NS_int;

class C {
  const int MAX = 1000;
  private int $prop = 10;

  public function setProp(int $val): void {
    $this->prop = $val;
  }

  public function getProp(): int {
    return $this->prop;
  }
}

function main(): void {
  var_dump(-PHP_INT_MAX - 1);
  var_dump(-PHP_INT_MAX - 1 - 1);		// wraps to max positive

  var_dump(PHP_INT_MAX);
  var_dump(PHP_INT_MAX + 1);			// wraps to min negative

  var_dump(PHP_INT_MAX/2 + PHP_INT_MAX);	// converts to float

  var_dump(PHP_INT_MIN);			// added in PHP7
}

/* HH_FIXME[1002] call to main in strict*/
main();
