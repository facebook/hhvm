<?hh // strict
<<__PHPStdLib>>
class Ignored {
  public function foo() : void {}

}


function test() : void {

  $x = (new Ignored());
  $x->foo();
}
