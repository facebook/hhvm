<?hh

class Foo {

  public function __construct(
    public int $prop,
   ) {}

}

function pure_lambda_not_inherited(): void {
  $f = (Foo $x)[] ==> {
    $x->prop = 4;
  };
}
