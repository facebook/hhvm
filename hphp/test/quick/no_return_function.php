<?hh

class Something {
  static public function thrower() {
    throw new Exception('heh');
  }

  static public function blah(?string $x = null) {
    if ($x) {
      try {
        self::thrower();
      } catch (Exception $l) {
        echo "caught!\n";
        echo "$x\n";
      }
    } else {
    }
  }

  static public function looper() {
    for (;;) {}
  }

  static public function blah2(?string $x = null) {
    if ($x) {
      return self::looper();
    }
    echo "not calling looper; this test would never stop ...\n";
    return 12;
  }
}

Something::blah();
Something::blah("asd");

Something::blah2();
