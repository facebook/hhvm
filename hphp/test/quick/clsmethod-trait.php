<?hh

trait t {
  public static function f() :mixed{
    a::priv();
  }
}

class a {
  use t;

  private static function priv() :mixed{
    echo "Private in a\n";
  }
}

<<__EntryPoint>> function main(): void {
  a::f();
  t::f();
}
