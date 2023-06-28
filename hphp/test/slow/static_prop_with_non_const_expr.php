<?hh

class C {
  static function f($x) :mixed{
    return $x;
  }

  public static $x = "foo" |> f($$);
}

