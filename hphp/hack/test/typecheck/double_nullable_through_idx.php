<?hh

class Foo {}

function getFoos(): dict<string, ?Foo> {
  return dict['test' => new Foo()];
}

function test(): void {
  $foos = getFoos();
  $foo = idx($foos, 'test');
  hh_show($foo);
}
