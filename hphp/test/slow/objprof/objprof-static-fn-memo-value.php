<?hh

// A is a normal class that contains static memoized methods
class A {
  <<__Memoize>>
  static function zeroParam1(): string {
    return str_repeat('a', 1000);
  }
  <<__Memoize>>
  static function zeroParam2(): string {
    return str_repeat('b', 1000);
  }
  <<__Memoize>>
  static function zeroParam3(): string {
    return str_repeat('c', 1000);
  }

}

// B is a singleton class that contains static memoized methods
class B {
  private static ?B $self = null;
  <<__Memoize>>
  static function zeroParam1(): string {
    return str_repeat('a', 1000);
  }
  <<__Memoize>>
  static function zeroParam2(): string {
    return str_repeat('b', 1000);
  }
  <<__Memoize>>
  static function zeroParam3(): string {
    return str_repeat('c', 1000);
  }

  static function get(): this {
    $b = self::$self;
    if ($b is null) {
      $b = new B();
      self::$self = $b;
    }
    return $b;
  }
}

function check_and_print($condition, $message) {
    if ($condition) {
        echo "Pass";
    } else {
        echo "Fail";
    }
    echo ": " . $message . "\n";
}

<<__EntryPoint>>
function main(): void {
    $a = new A();
    A::zeroParam1();
    A::zeroParam2();
    A::zeroParam3();

    $b = B::get();
    B::zeroParam1();
    B::zeroParam2();
    B::zeroParam3();
    $prof = objprof_get_data_extended();
    $prof_per_prop = objprof_get_data_extended(OBJPROF_FLAGS_PER_PROPERTY);

    // The normalized size of A::Static should be greater than 3000 bytes due to 3x memo values
    check_and_print($prof["A::Static"]["bytes_normalized"] > 3000, "The normalized size of A should be greater than 3000");
    // The normalized size of B::Static should be greater than 3000 bytes due to 3x memo values
    check_and_print($prof["A::Static"]["bytes_normalized"] > 3000, "The normalized size of A should be greater than 3000");

    // Each of the static memoized functions should have their own entry in the per property map
    check_and_print($prof_per_prop["A::zeroParam1"]["bytes_normalized"] > 1000, "The normalized size of A::zeroParam1 should be greater than 1000");
    check_and_print($prof_per_prop["A::zeroParam2"]["bytes_normalized"] > 1000, "The normalized size of A::zeroParam2 should be greater than 1000");
    check_and_print($prof_per_prop["A::zeroParam3"]["bytes_normalized"] > 1000, "The normalized size of A::zeroParam3 should be greater than 1000");
    check_and_print($prof_per_prop["B::zeroParam1"]["bytes_normalized"] > 1000, "The normalized size of B::zeroParam1 should be greater than 1000");
    check_and_print($prof_per_prop["B::zeroParam2"]["bytes_normalized"] > 1000, "The normalized size of B::zeroParam2 should be greater than 1000");
    check_and_print($prof_per_prop["B::zeroParam3"]["bytes_normalized"] > 1000, "The normalized size of B::zeroParam3 should be greater than 1000");

}
