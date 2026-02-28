<?hh


class Ok implements HH\ModuleAttribute {
  public function foo(): string {
    return "aoidwjad";
  }
}
<<Ok>>
new module foo {}

<<__EntryPoint>>
function test(): void {

  $x = new ReflectionModule("foo");
  var_dump($x->getAttributes());
  var_dump($x->getAttributeClass("Ok"));
  var_dump($x->getAttributeClass("Ok")->foo());
}
