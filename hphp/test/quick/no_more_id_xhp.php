<?hh

class :foo {
  public function bar() :mixed{ return 'baz'; }
}
<<__EntryPoint>> function main(): void {
var_dump((<foo />)->bar());
}
