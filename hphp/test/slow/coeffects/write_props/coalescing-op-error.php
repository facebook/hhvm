<?hh

class Foo {

  public function __construct(
    public string $prop_str,
   ) {}

}
function write_props_function(Foo $x)[write_props]: void {
  $x->prop_str ??= 'default'; // No error
}

function pure_function(Foo $x)[] : void {
  $x->prop_str ??= 'default'; // Error
}
