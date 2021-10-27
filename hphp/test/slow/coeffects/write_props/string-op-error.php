<?hh

class Foo {

  public function __construct(
    public string $prop_str,
   ) {}

}

function pure_function(Foo $x)[] : void {
  $x->prop_str .= 'more text';
}
