<?hh
class Bar {}
class Foo {
  public function __construct(public mixed $foo) {
  }
}
<<__EntryPoint>>
function test() : void {
  $y = readonly new Bar();
  new Foo($y); // error
}
