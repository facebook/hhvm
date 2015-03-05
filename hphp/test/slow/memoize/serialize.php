<?hh


class C1 {
  const PURPOSE = 'Base';

  <<__Memoize>>
  public function f(int $x): int {
    static $y = 0;
    return $x + 1 + ($y++);
  }
}

class C2 {
  const PURPOSE = 'WithoutMemoize';

  public function f(int $x): int {
    static $y = 0;
    return $x + 5 + ($y++);
  }
}

class C3 {
  const PURPOSE = 'WithDifferentCode';

  <<__Memoize>>
  public function f(int $x): int {
    static $y = 0;
    return $x + 5 + ($y++);
  }
}

class C4 {
  const PURPOSE = 'WithDifferentParam';

  <<__Memoize>>
  public function f($x): int {
    static $y = 0;
    return (int)$x + 1 + ($y++);
  }
}

class C5 {
  const PURPOSE = 'WithMoreParams';

  <<__Memoize>>
  public function f(int $x, int $incr = 1): int {
    static $y = 0;
    return $x + $incr + ($y++);
  }
}

class C6 {
  const PURPOSE = 'WithNoParams';

  <<__Memoize>>
  public function f(): int {
    static $y = 0;
    return ($y++);
  }
}

function test_unserialize($class, $s) {
  invariant(strlen($class) === strlen(C1::class),
            'string lengths must match');

  echo '== unserialize', ' (', $class::PURPOSE, ')',
    ' ==', "\n";

  // changing the class in the string to simulate deserialization into a
  // PHP with a different implementation of C1
  $s = strtr($s, array( C1::class => $class));
  var_dump($s);

  $un = unserialize($s);
  var_dump($un->f(10));
  var_dump($un->f(10));
  var_dump($un->f(20));
  var_dump($un->f(20));
  var_dump($un->f(30));
  var_dump($un->f(30));
}


function main() {
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

main();
