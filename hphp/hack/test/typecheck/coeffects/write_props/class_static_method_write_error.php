<?hh

class Foo {

  public function __construct(
    public int $prop_int,
   ) {}

  public static function pure_static_method(Foo $x)[] : void {
    $x->prop_int = 4; // Error
  }

}
