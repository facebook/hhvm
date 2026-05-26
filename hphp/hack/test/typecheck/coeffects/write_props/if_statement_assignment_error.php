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

function pure_function(Bar $x)[] : void {
  if ($x->prop_bool = true) {} // write_props error
}

function write_props_function(Bar $x)[write_props] : void {
  if ($x->prop_bool = true) {} // No write_props error
}
