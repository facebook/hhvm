<?hh

class a {
  private static function priv() :mixed{
    echo "Private in a\n";
  }

  public static function pub() :mixed{
    a::priv();
  }
}

<<__EntryPoint>> function main(): void {
  a::pub();
  a::priv();
}
