<?hh

class Foo {

  public function __construct(
    public dict<string, string> $prop_dict,
   ) {}

}

function pure_function(Foo $x)[] : void {
  $x->prop_dict['a'] = 'b'; // Error
}
