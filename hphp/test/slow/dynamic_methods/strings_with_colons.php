<?php

class W {
  public function dyn() {
    var_dump(get_called_class());
    var_dump(isset($this) ? $this : null);
  }
  public static function stc() {
    var_dump(get_called_class());
  }
}

class X extends W {
  public function bar($s) {
    $s();
  }
}
class Y extends X {}

function test($str) {
  X::bar($str);
  (new X)->bar($str);
  Y::bar($str);
  (new Y)->bar($str);
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

test_all("stc");
test_all("dyn");
X::bar("W::non");
