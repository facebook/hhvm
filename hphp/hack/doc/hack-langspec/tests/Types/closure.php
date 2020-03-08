<?hh // strict

namespace NS_closure;

class C {
//  const (function (): void) T = // *** no way to write a closure constant expression

  private (function (): void) $prop;

  public function setProp((function (): void) $val): void {
    $this->prop = $val;
  }

  public function getProp(): (function (): void) {
    return $this->prop;
  }

  public function __construct() {
    $this->prop = function (): void { echo "Hi there!\n"; };
  }
}

function doit(int $iValue, (function (int): int) $process): int {
  return $process($iValue);
}

function main(): void {
  $result = doit(5, function ($p) { return $p * 2; });	// doubles 5
  var_dump($result);
  $result = doit(5, function ($p) { return $p * $p; });	// squares 5
  var_dump($result);
}

/* HH_FIXME[1002] call to main in strict*/
main();
