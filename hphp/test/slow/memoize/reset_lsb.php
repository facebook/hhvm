<?hh

class A {
  public static int $runCount = 0;
  public static vec<string> $sideEffects = vec[];

  <<__MemoizeLSB>>
  public static function foo1(): string {
    $r = self::$runCount . " " . static::class . "::foo1()";
    self::$sideEffects[] = $r;
    return $r;
  }

  <<__MemoizeLSB>>
  public static function foo2(int $arg): string {
    $r = self::$runCount . " " . static::class . "::foo2($arg)";
    self::$sideEffects[] = $r;
    return $r;
  }

}

class B extends A { }
class C extends B { }

function run(?string $cls, ?string $func = null) :mixed{
  if ($cls !== null) {
    $ok = HH\clear_lsb_memoization($cls, $func) ? "T" : "F";
  } else {
    $ok = "-";
  }
  A::$sideEffects = vec[];
  $values = vec[
    A::foo1(),
    B::foo1(),
    C::foo1(),
    A::foo2(0),
    B::foo2(0),
    C::foo2(0),
    A::foo2(1),
    B::foo2(1),
    C::foo2(1),
  ];
  echo $ok . " " . \implode(", ", $values) .
    " | " . \implode(", ", A::$sideEffects) . "\n";
  A::$runCount++;
}

function main() :mixed{
  run(null);

  run("A", "foo1");
  run("B", "foo1");
  run("C", "foo1");

  run("A", "foo2");
  run("B", "foo2");
  run("C", "foo2");

  run("A");
  run("B");
  run("C");
}


<<__EntryPoint>>
function main_reset_lsb() :mixed{
main();
}
