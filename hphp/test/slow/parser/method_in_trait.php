<?hh

trait T {
  static function fun($x) {
    echo "T::fun claims to be `" . __METHOD__ . "'\n";
    if ($x) {
      function bar() {}
    }
    echo "T::fun claims to be `" . __METHOD__ . "'\n";
  }
}

class C {
  use T;
}

C::fun(false);
T::fun(false);
