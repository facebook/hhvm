<?hh

class Foo {

  public function __construct(
    public int $prop_int,
   ) {}

}

function pure_function(Foo $x)[] : void {
  $x->prop_int <<= 4;
}
