<?hh

class Foo {

  public function __construct(
    public num $prop_num,
   ) {}

}
function write_props_function(Foo $x)[write_props] : void {
  $x->prop_num **= 4; // No error
}

function pure_function(Foo $x)[] : void {
  $x->prop_num **= 4; // Error
}
