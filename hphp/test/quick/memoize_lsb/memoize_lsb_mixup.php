<?hh

class A {
  public static int $counter = 0;

  <<__Memoize>>
  public static function memo1(): int {
    return self::$counter++;
  }

  <<__MemoizeLSB>>
  public static function lsb1(): int {
    return self::$counter++;
  }
}

class B extends A {
  <<__Memoize>>
  public static function memo2(): int {
    return self::$counter++;
  }

  <<__MemoizeLSB>>
  public static function lsb2(): int {
    return self::$counter++;
  }
}

class C extends B {
  <<__Memoize>>
  public static function memo3(): int {
    return self::$counter++;
  }

  <<__MemoizeLSB>>
  public static function lsb3(): int {
    return self::$counter++;
  }
}


function main() :mixed{
  // These are all possibilities, sorted randomly,
  // with the expected value determined by hand.
  $results = vec[
    B::lsb2() === 0,
    C::lsb2() === 1,
    B::memo1() === 2,
    A::lsb1() === 3,
    C::lsb3() === 4,
    C::memo1() === 2,
    B::lsb1() === 5,
    A::memo1() === 2,
    C::memo2() === 6,
    B::memo2() === 6,
    C::memo3() === 7,
    C::lsb1() === 8,

    A::$counter == 9,
  ];
  var_dump($results);
}
<<__EntryPoint>> function main_entry(): void {
main();
main();
}
