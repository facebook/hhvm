<?hh

class Base {
  public static vec<string> $side_effects = vec[];

  <<__MemoizeLSB>>
  public static function memo1(): string {
    $ret = static::class . "1";
    self::$side_effects[] = $ret;
    return $ret;
  }

  <<__MemoizeLSB>>
  public static function memo2(): string {
    $ret = static::class . "2";
    self::$side_effects[] = $ret;
    return $ret;
  }

}

class A extends Base {

  <<__MemoizeLSB>>
  public static function memo3(): string {
    $ret = static::class . "3";
    self::$side_effects[] = $ret;
    return $ret;
  }

  <<__MemoizeLSB>>
  public static function memo4(): string {
    $ret = static::class . "4";
    self::$side_effects[] = $ret;
    return $ret;
  }

}

class B extends Base {
  <<__MemoizeLSB>>
  public static function memo5(): string {
    $ret = static::class . "5";
    self::$side_effects[] = $ret;
    return $ret;
  }
}

function main() :mixed{
  Base::$side_effects = vec[];
  $results = vec[
    B::memo5(),
    B::memo2(),
    B::memo1(),

    A::memo4(),
    A::memo3(),
    A::memo2(),
    A::memo1(),

    Base::memo2(),
    Base::memo1()
  ];
  var_dump($results);
  var_dump(Base::$side_effects);
}
<<__EntryPoint>> function main_entry(): void {
main();
main();
}
