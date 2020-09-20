<?hh

class W {




  public static function stc() {
    var_dump(static::class);
  }
}

class X extends W {
  public function ibar($s) {
    $s();
  }
  public static function sbar($s) {
    $s();
  }
}
class Y extends X {}

function test($str) {
  X::sbar($str);
  (new X)->ibar($str);
  Y::sbar($str);
  (new Y)->ibar($str);
}

function test_all($str) {
  test("X::$str");
  echo "----\n";
  test("self::$str");
  echo "----\n";
  test("parent::$str");
  echo "----\n";
  test("static::$str");
  echo "====\n";
}


<<__EntryPoint>>
function main_strings_with_colons() {
test_all("stc");

X::sbar("W::non");
}
