<?hh

class Foo {}
<<__EntryPoint>>
function foo(): void {
  $x = readonly new Foo();
  var_dump(\spl_object_hash($x));
}
