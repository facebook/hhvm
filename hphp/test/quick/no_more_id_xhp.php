<?hh

class :foo {
  public function bar() { return 'baz'; }
}
<<__EntryPoint>> function main(): void {
var_dump((<foo />)->bar());
}
