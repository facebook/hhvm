<?hh


class C1 {
  const PURPOSE = 'Base';

  private static $fY = 0;

  <<__Memoize>>
  public function f(int $x): int {
    return $x + 1 + (self::$fY++);
  }
}

class C2 {
  const PURPOSE = 'WithoutMemoize';

  private static $fY = 0;

  public function f(int $x): int {
    return $x + 5 + (self::$fY++);
  }
}

class C3 {
  const PURPOSE = 'WithDifferentCode';

  private static $fY = 0;

  <<__Memoize>>
  public function f(int $x): int {
    return $x + 5 + (self::$fY++);
  }
}

class C4 {
  const PURPOSE = 'WithDifferentParam';

  private static $fY = 0;

  <<__Memoize>>
  public function f($x): int {
    return (int)$x + 1 + (self::$fY++);
  }
}

class C5 {
  const PURPOSE = 'WithMoreParams';

  private static $fY = 0;

  <<__Memoize>>
  public function f(int $x, int $incr = 1): int {
    return $x + $incr + (self::$fY++);
  }
}

class C6 {
  const PURPOSE = 'WithNoParams';

  private static $fY = 0;

  <<__Memoize>>
  public function f(): int {
    return (self::$fY++);
  }
}

function test_unserialize($class, $s) :mixed{
  invariant(strlen($class) === strlen(C1::class),
            'string lengths must match');

  echo '== unserialize', ' (', $class::PURPOSE, ')',
    ' ==', "\n";

  // changing the class in the string to simulate deserialization into a
  // PHP with a different implementation of C1
  $s = strtr($s, dict[ C1::class => $class]);
  var_dump($s);

  $un = unserialize($s);
  var_dump($un->f(10));
  var_dump($un->f(10));
  var_dump($un->f(20));
  var_dump($un->f(20));
  var_dump($un->f(30));
  var_dump($un->f(30));
}


function main() :mixed{
  echo '== no serialization ==', "\n";
  $inst = new C1();
  var_dump($inst->f(10));
  var_dump($inst->f(10));
  var_dump($inst->f(20));
  var_dump($inst->f(20));

  $s = serialize($inst);
  var_dump($s);

  test_unserialize(C1::class, $s);
  test_unserialize(C2::class, $s);
  test_unserialize(C3::class, $s);
  test_unserialize(C4::class, $s);
  test_unserialize(C5::class, $s);
  test_unserialize(C6::class, $s);
}


<<__EntryPoint>>
function main_serialize() :mixed{
main();
}
