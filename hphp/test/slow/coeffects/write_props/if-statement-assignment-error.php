<?hh

class Bar {

  public function __construct(
    public bool $prop_bool,
  ) {}
}

class Foo {

  public function __construct(
    public int $prop_int,
   ) {}

}
function write_props_function(Foo $x)[write_props] : void {
  if ($x->prop_int = 5) {} // No error
}

function pure_function(Foo $x)[] : void {
  if ($x->prop_int = 5) {} // Error
}
