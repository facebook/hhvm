<?hh

class C<reify Ta, reify Tb> {
  public static function f() :mixed{
    new self();
  }
}

