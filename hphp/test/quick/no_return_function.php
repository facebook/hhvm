<?hh

class Something {
  static public function thrower() :mixed{
    throw new Exception('heh');
  }

  static public function blah(?string $x = null) :mixed{
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

  static public function looper() :mixed{
    for (;;) {}
  }

  static public function blah2(?string $x = null) :mixed{
    if ($x) {
      return self::looper();
    }
    echo "not calling looper; this test would never stop ...\n";
    return 12;
  }
}
<<__EntryPoint>> function main(): void {
Something::blah();
Something::blah("asd");

Something::blah2();
}
