<?hh

class Foo {

  public function __construct(
    public int $prop,
   ) {}

  public function pure_lambda_not_inherited_method(): void {
    $f = (Foo $x)[] ==> {
      $x->prop = 4;
    };
  }

}

function pure_lambda_not_inherited_function(): void {
  $f = (Foo $x)[] ==> {
    $x->prop = 4;
  };
}
