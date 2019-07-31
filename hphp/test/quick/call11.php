<?hh
class C {
  public function __call($fn, $args) {
    echo "C::__call\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
  public static function test() {
    // FCallClsMethodD
    C::foo("a", "b", "c", "d");

    // FCallClsMethod
    $cls = 'C';
    $cls::foo("a", "b", "c", "d");
    $fn = 'foo';
    C::$fn("a", "b", "c", "d");
    $fn = 'foo';
    $cls::$fn("a", "b", "c", "d");

    // FCallClsMethodSD
    self::foo("a", "b", "c", "d");
  }
}

<<__EntryPoint>>
function main() {
  C::test();
}
