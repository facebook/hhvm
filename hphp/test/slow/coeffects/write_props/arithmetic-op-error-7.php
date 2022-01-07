<?hh

class Foo {

  public function __construct(
    public int $prop_int,
   ) {}

}
function write_props_function(Foo $x)[write_props] : void {
  $x->prop_int %= 4; // No error
}

function pure_function(Foo $x)[] : void {
  $x->prop_int %= 4; // Error
}
