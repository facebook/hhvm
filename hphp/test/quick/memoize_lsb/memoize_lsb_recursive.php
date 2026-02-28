<?hh


class A {
  public static vec<int> $side_effects = vec[];

  public static int $s0 = 1;
  public static int $s1 = 1;

  <<__MemoizeLSB>>
  public static function fibonacci(int $n): int {
    self::$side_effects[] = $n;
    if ($n === 0) return static::$s0;
    if ($n === 1) return static::$s1;
    return static::fibonacci($n - 1) + static::fibonacci($n - 2);
  }
}

class B extends A {
  public static int $s0 = 0;
  public static int $s1 = 1;
}


function doA(int $n) :mixed{
  A::$side_effects = vec[];
  var_dump(A::fibonacci($n));
  var_dump(A::$side_effects);
}

function doB(int $n) :mixed{
  B::$side_effects = vec[];
  var_dump(B::fibonacci($n));
  var_dump(B::$side_effects);
}
<<__EntryPoint>> function main(): void {
doA(10);
doA(5);
doA(15);


doB(10);
doB(5);
doB(15);
}
