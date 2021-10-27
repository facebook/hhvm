<?hh

class Foo {

  public function __construct(
    public num $prop_num,
   ) {}

}

function pure_function(Foo $x)[] : void {
  $x->prop_num **= 4;
}
