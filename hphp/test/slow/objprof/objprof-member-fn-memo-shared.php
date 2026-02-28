<?hh


class A {
  <<__Memoize>>
  function zeroParam1(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam2(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam3(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam4(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam5(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam6(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam7(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam8(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam9(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam10(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam11(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function zeroParam12(): string {
    return str_repeat('a', 100);
  }
  <<__Memoize>>
  function oneParam1($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam2($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam3($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam4($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam5($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam6($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam7($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam8($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam9($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function oneParam10($a): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams1($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams2($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams3($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams4($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams5($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams6($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams7($a, $b): string {
    return str_repeat($a, 100);
  }
    <<__Memoize>>
  function twoParams8($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams9($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function twoParams10($a, $b): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function threeParams1($a, $b, $c): string {
    return str_repeat($a, 100);
  }
  <<__Memoize>>
  function fourParams1($a, $b, $c, $d): string {
    return str_repeat($a, 100);
  }
  private dict<int, string> $myDict = dict[];
  public function initDict(): void {
    for ($i = 0; $i < 100; $i++) {
      $this->myDict[$i] = 'ABC';
    }
  }
  <<__Memoize>>
  public function getDictOfStrings(): void {
    $t = dict[];
    for ($i = 0; $i < 100; $i++) {
      $t[$i] = str_repeat('k', 100);
    }
    return $t;
  }
  public function getStr(int $len): string {
    return str_repeat('X', $len);
  }
  private $myString;
  public function __construct() {
    $this->myString = $this->getStr(100);
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
    $a = __hhvm_intrinsics\launder_value(new A());
    // called 6 times, but 0 parameter, so only 1 memocache should be taken
    $a->zeroParam1();
    $a->zeroParam1();
    $a->zeroParam1();
    $a->zeroParam1();
    $a->zeroParam1();
    $a->zeroParam1();
    $a->zeroParam2();
    $a->zeroParam3();
    $a->zeroParam4();
    $a->zeroParam5();
    $a->zeroParam6();
    $a->zeroParam7();
    $a->zeroParam8();
    $a->zeroParam9();
    $a->zeroParam10();
    $a->zeroParam11();
    $a->zeroParam12();
    $a->oneParam1('b');
    $a->oneParam2('c');
    $a->oneParam3('c');
    $a->oneParam4('b');
    $a->oneParam5('c');
    $a->oneParam6('c');
    $a->oneParam7('b');
    $a->oneParam8('c');
    $a->oneParam9('c');
    $a->oneParam10('b');
    $a->oneParam1('c');
    // we call this 5 times but with only two unique values
    $a->oneParam10('c');
    $a->oneParam10('b');
    $a->oneParam10('c');
    $a->oneParam10('c');
    // we call each of the two and three parameter functions 3 times, but with the same values
    // so expected instances below will be only 1
    $a->twoParams1('c','d');
    $a->twoParams2('c','d');
    $a->twoParams3('c','d');
    $a->twoParams4('c','d');
    $a->twoParams5('c','d');
    $a->twoParams6('c','d');
    $a->twoParams7('c','d');
    $a->twoParams8('c','d');
    $a->twoParams9('c','d');
    $a->twoParams10('c','d');
    $a->threeParams1('c','d','e');
    $a->fourParams1('c','d','e','f');
    $a->twoParams1('c','d');
    $a->twoParams2('c','d');
    $a->twoParams3('c','d');
    $a->twoParams4('c','d');
    $a->twoParams5('c','d');
    $a->twoParams6('c','d');
    $a->twoParams7('c','d');
    $a->twoParams8('c','d');
    $a->twoParams9('c','d');
    $a->twoParams10('c','d');
    $a->threeParams1('c','d','e');
    $a->fourParams1('c','d','e','f');
    $a->twoParams1('c','d');
    $a->twoParams2('c','d');
    $a->twoParams3('c','d');
    $a->twoParams4('c','d');
    $a->twoParams5('c','d');
    $a->twoParams6('c','d');
    $a->twoParams7('c','d');
    $a->twoParams8('c','d');
    $a->twoParams9('c','d');
    $a->twoParams10('c','d');
    $a->threeParams1('c','d','e');
    $a->fourParams1('c','d','e','f');
    $a->initDict();
    $a->getDictOfStrings();
    $prof = objprof_get_data_extended();
    $prof_per_prop = objprof_get_data_extended(OBJPROF_FLAGS_PER_PROPERTY);
    __hhvm_intrinsics\launder_value($a);

    check_and_print($prof["A"]["bytes_normalized"] > 100, "The normalized size of A should be greater than 100");
    echo "\n";
    // Total number of instances should be 1 because we are in default mode
    check_and_print($prof["A"]["instances"] == 1, "Total number of instances of A should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam1"]["bytes_normalized"] > 100, "The normalized size of zeroParam1 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam1"]["instances"] == 1, "Total number of instances of zeroParam1 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam2"]["bytes_normalized"] > 100, "The normalized size of zeroParam2 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam2"]["instances"] == 1, "Total number of instances of zeroParam2 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam3"]["bytes_normalized"] > 100, "The normalized size of zeroParam3 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam3"]["instances"] == 1, "Total number of instances of zeroParam3 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam4"]["bytes_normalized"] > 100, "The normalized size of zeroParam4 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam4"]["instances"] == 1, "Total number of instances of zeroParam4 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam5"]["bytes_normalized"] > 100, "The normalized size of zeroParam5 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam5"]["instances"] == 1, "Total number of instances of zeroParam5 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam6"]["bytes_normalized"] > 100, "The normalized size of zeroParam6 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam6"]["instances"] == 1, "Total number of instances of zeroParam6 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam7"]["bytes_normalized"] > 100, "The normalized size of zeroParam7 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam7"]["instances"] == 1, "Total number of instances of zeroParam7 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam8"]["bytes_normalized"] > 100, "The normalized size of zeroParam8 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam8"]["instances"] == 1, "Total number of instances of zeroParam8 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam9"]["bytes_normalized"] > 100, "The normalized size of zeroParam9 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam9"]["instances"] == 1, "Total number of instances of zeroParam9 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam10"]["bytes_normalized"] > 100, "The normalized size of zeroParam10 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam10"]["instances"] == 1, "Total number of instances of zeroParam10 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam11"]["bytes_normalized"] > 100, "The normalized size of zeroParam11 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam11"]["instances"] == 1, "Total number of instances of zeroParam11 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam1"]["bytes_normalized"] > 100, "The normalized size of oneParam1 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam1"]["instances"] == 2, "Total number of instances of oneParam1 should be 2");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam2"]["bytes_normalized"] > 100, "The normalized size of oneParam2 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam2"]["instances"] == 1, "Total number of instances of oneParam2 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam3"]["bytes_normalized"] > 100, "The normalized size of oneParam3 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam3"]["instances"] == 1, "Total number of instances of oneParam3 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam4"]["bytes_normalized"] > 100, "The normalized size of oneParam4 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam4"]["instances"] == 1, "Total number of instances of oneParam4 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam5"]["bytes_normalized"] > 100, "The normalized size of oneParam5 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam5"]["instances"] == 1, "Total number of instances of oneParam5 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam6"]["bytes_normalized"] > 100, "The normalized size of oneParam6 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam6"]["instances"] == 1, "Total number of instances of oneParam6 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam7"]["bytes_normalized"] > 100, "The normalized size of oneParam7 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam7"]["instances"] == 1, "Total number of instances of oneParam7 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam8"]["bytes_normalized"] > 100, "The normalized size of oneParam8 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam8"]["instances"] == 1, "Total number of instances of oneParam8 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam9"]["bytes_normalized"] > 100, "The normalized size of oneParam9 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam9"]["instances"] == 1, "Total number of instances of oneParam9 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam10"]["bytes_normalized"] > 100, "The normalized size of oneParam10 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::oneParam10"]["instances"] == 2, "Total number of instances of oneParam10 should be 2");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams1"]["bytes_normalized"] > 100, "The normalized size of twoParams1 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams1"]["instances"] == 1, "Total number of instances of twoParams1 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams2"]["bytes_normalized"] > 100, "The normalized size of twoParams2 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams2"]["instances"] == 1, "Total number of instances of twoParams2 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams3"]["bytes_normalized"] > 100, "The normalized size of twoParams3 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams3"]["instances"] == 1, "Total number of instances of twoParams3 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams4"]["bytes_normalized"] > 100, "The normalized size of twoParams4 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams4"]["instances"] == 1, "Total number of instances of twoParams4 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams5"]["bytes_normalized"] > 100, "The normalized size of twoParams5 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams5"]["instances"] == 1, "Total number of instances of twoParams5 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams6"]["bytes_normalized"] > 100, "The normalized size of twoParams6 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams6"]["instances"] == 1, "Total number of instances of twoParams6 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams7"]["bytes_normalized"] > 100, "The normalized size of twoParams7 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams7"]["instances"] == 1, "Total number of instances of twoParams7 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams8"]["bytes_normalized"] > 100, "The normalized size of twoParams8 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams8"]["instances"] == 1, "Total number of instances of twoParams8 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams9"]["bytes_normalized"] > 100, "The normalized size of twoParams9 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams9"]["instances"] == 1, "Total number of instances of twoParams9 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams10"]["bytes_normalized"] > 100, "The normalized size of twoParams10 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::twoParams10"]["instances"] == 1, "Total number of instances of twoParams10 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::threeParams1"]["bytes_normalized"] > 100, "The normalized size of threeParams1 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::threeParams1"]["instances"] == 1, "Total number of instances of threeParams1 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::fourParams1"]["bytes_normalized"] > 100, "The normalized size of fourParams1 should be greater than 100");
    echo "\n";
    check_and_print($prof_per_prop["A::fourParams1"]["instances"] == 1, "Total number of instances of fourParams1 should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::getDictOfStrings"]["bytes_normalized"] > 10000, "The normalized size of getDictOfStrings should be greater than 10000");
    echo "\n";
    check_and_print($prof_per_prop["A::getDictOfStrings"]["instances"] == 1, "Total number of instances of getDictOfStrings should be 1");
    echo "\n";

}
