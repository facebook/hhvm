<?hh

// A is a normal class that contains static memoized methods
class A {
  <<__Memoize>>
  static function oneParam1($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam2($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam3($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam4($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam5($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam6($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam7($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam8($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam9($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam10($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam11($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam12($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam13($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function elevenParam($a, $b, $c, $d, $e, $f, $g, $h, $i, $j, $k): string {
    return str_repeat($a, 1000);
  }

}

// B is a singleton class that contains static memoized methods
class B {
  private static ?B $self = null;
  <<__Memoize>>
  static function oneParam1($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam2($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  static function oneParam3($a): string {
    return str_repeat($a, 1000);
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
    A::oneParam1('a');
    A::oneParam2('a');
    A::oneParam3('a');
    A::oneParam4('a');
    A::oneParam5('a');
    A::oneParam6('a');
    A::oneParam7('a');
    A::oneParam8('a');
    A::oneParam9('a');
    A::oneParam10('a');
    A::oneParam11('a');
    A::oneParam12('a');
    A::oneParam13('a');
    // adding this as a test case that exceeds max keysize for memoization (i.e. buckets into max)
    A::elevenParam('a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k');


    $b = B::get();
    B::oneParam1('a');
    B::oneParam2('a');
    B::oneParam3('a');
    $prof = objprof_get_data_extended();
    $prof_per_prop = objprof_get_data_extended(OBJPROF_FLAGS_PER_PROPERTY);

    // The normalized size of A::Static should be greater than 14000 bytes due to 14x memo caches
    check_and_print($prof["A::Static"]["bytes_normalized"] > 14000, "The normalized size of A should be greater than 14000");
    // The normalized size of B::Static should be greater than 3000 bytes due to 3x memo caches
    check_and_print($prof["A::Static"]["bytes_normalized"] > 3000, "The normalized size of A should be greater than 3000");
    // Each of the static memoized functions should have their own entry in the per property map
    check_and_print($prof_per_prop["A::oneParam1"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam1 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam2"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam2 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam3"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam3 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam4"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam4 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam5"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam5 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam6"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam6 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam7"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam7 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam8"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam8 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam9"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam9 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam10"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam10 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam11"]["bytes_normalized"] > 1000, "The normalized size of A::onePara11 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam12"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam12 should be greater than 1000");
    check_and_print($prof_per_prop["A::oneParam13"]["bytes_normalized"] > 1000, "The normalized size of A::oneParam13 should be greater than 1000");
    check_and_print($prof_per_prop["A::elevenParam"]["bytes_normalized"] > 1000, "The normalized size of A::elevenParam should be greater than 1000");

    check_and_print($prof_per_prop["B::oneParam1"]["bytes_normalized"] > 1000, "The normalized size of B::oneParam1 should be greater than 1000");
    check_and_print($prof_per_prop["B::oneParam2"]["bytes_normalized"] > 1000, "The normalized size of B::oneParam2 should be greater than 1000");
    check_and_print($prof_per_prop["B::oneParam3"]["bytes_normalized"] > 1000, "The normalized size of B::oneParam3 should be greater than 1000");

}
