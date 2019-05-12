<?hh

trait t {
  public static function f() {
    a::priv();
  }
}

class a {
  use t;

  private static function priv() {
    echo "Private in a\n";
  }
}

<<__EntryPoint>> function main(): void {
  a::f();
  t::f();
}
