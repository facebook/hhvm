<?hh

class Foo {

  public function __construct(
    public dict<string, string> $prop_dict,
   ) {}

}

function write_props_function(Foo $x)[write_props] : void {
  $x->prop_dict['a'] = 'b'; // No error
}

function pure_function(Foo $x)[] : void {
  $x->prop_dict['a'] = 'b'; // Error
}
