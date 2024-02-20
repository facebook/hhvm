<?hh


class A {
  <<__Memoize>>
  function zeroParam(): string {
    return str_repeat('a', 1000);
  }
  <<__Memoize>>
  function oneParam($a): string {
    return str_repeat($a, 1000);
  }
  <<__Memoize>>
  function twoParams($a, $b): string {
    return str_repeat($a, 1000);
  }
  private dict<int, string> $myDict = dict[];
  public function initDict(): void {
    for ($i = 0; $i < 1000; $i++) {
      $this->myDict[$i] = 'ABC';
    }
  }
  public function getStr(int $len): string {
    return str_repeat('X', $len);
  }
  private $myString;
  public function __construct() {
    $this->myString = $this->getStr(1000);
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
    $c = $a->zeroParam();
    $b = $a->oneParam('b');
    $a->oneParam('c');
    $a->twoParams('c','d');
    $a->initDict();
    $prof = objprof_get_data_extended();
    $prof_per_prop = objprof_get_data_extended(OBJPROF_FLAGS_PER_PROPERTY);
    // The normalized size of A should be greater than 5000 bytes due to 3x memo caches 1000 bytes each
    // plus string and dict properties
    check_and_print($prof["A"]["bytes_normalized"] > 5000, "The normalized size of A should be greater than 5000");
    echo "\n";

    check_and_print($prof["A"]["instances"] == 1, "Total number of instances of A should be 1");
    echo "\n";
    check_and_print($prof_per_prop["A::zeroParam"]["bytes_normalized"] > 1000, "The normalized size of zeroParam should be greater than 1000");
    echo "\n";
    $a = __hhvm_intrinsics\launder_value($a);
    check_and_print($prof_per_prop["A::oneParam"]["bytes_normalized"] > 1000, "The normalized size of oneParam should be greater than 1000");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams"]["bytes_normalized"] > 1000, "The normalized size of twoParams should be greater than 1000");
    echo "\n";

    check_and_print($prof_per_prop["A::myString"]["bytes_normalized"] > 1000, "The normalized size of myString should be greater than 1000");
    echo "\n";

    check_and_print($prof_per_prop["A::myDict"]["bytes_normalized"] > 1000, "The normalized size of myDict should be greater than 1000");
    echo "\n";

    check_and_print($prof_per_prop["A::zeroParam"]["instances"] == 1, "Total number of instances of zeroParam should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::oneParam"]["instances"] == 2, "Total number of instances of oneParam should be 2");
    echo "\n";

    check_and_print($prof_per_prop["A::twoParams"]["instances"] == 1, "Total number of instances of twoParams should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::myString"]["instances"] == 1, "Total number of instances of myString should be 1");
    echo "\n";

    check_and_print($prof_per_prop["A::myDict"]["instances"] == 1, "Total number of instances of myDict should be 1");
    echo "\n";
}
