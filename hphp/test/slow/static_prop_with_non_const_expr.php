<?hh

class C {
  static function f($x) {
    return $x;
  }

  static $x = "foo" |> f($$);
}

<<__EntryPoint>> function main(): void {}
