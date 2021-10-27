<?hh

class Foo {

  public function __construct(
    public int $prop,
   ) {}

}

function defaults_lambda_inherited(): void {
  $f = (Foo $x) ==> {
    $x->prop = 4;
  };
}

function defaults_lambda_not_inherited()[]: void {
  $f = (Foo $x)[defaults] ==> {
    $x->prop = 4;
  };
}

<<__EntryPoint>>
function main(): void {

  defaults_lambda_inherited();
  defaults_lambda_not_inherited();

  echo "Done\n";

}
