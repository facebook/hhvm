<?hh

class Foo {
  function method() {
    $other = $this;
  }
}
<<__EntryPoint>>
function main_entry(): void {

  $object = new Foo;
  $object->prop = "Hello\n";

  $object->method();

  $object->prop2 = "\tThere";
  $object->method();
}
