<?hh

class C {
  static function f($x) {
    return $x;
  }

  public static $x = "foo" |> f($$);
}

