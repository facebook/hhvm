<?hh // strict

namespace NS_array_literals;

class X {
  private array<int> $prop10 = array();
  private array<mixed> $prop11 = array(10, "red", true);
  private array<string, mixed> $prop12 = array('a' => 10, 'r' => array(20,30));

  private array<int> $prop20 = [];
  private array<mixed> $prop21 = [10, "red", true];
  private array<string, mixed> $prop22 = ['a' => 10, 'r' => [20,30]];
}

function main(): void {
  $x = new X();
  var_dump($x);
}

/* HH_FIXME[1002] call to main in strict*/
main();
