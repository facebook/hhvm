<?hh

class a {
  private static function priv() {
    echo "Private in a\n";
  }

  public static function pub() {
    a::priv();
  }
}

<<__EntryPoint>> function main(): void {
  a::pub();
  a::priv();
}
