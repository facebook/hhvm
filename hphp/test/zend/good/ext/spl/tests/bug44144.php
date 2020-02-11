<?hh
class Foo {
  public function nonstaticMethod() {}
}
<<__EntryPoint>> function main(): void {
$foo = new Foo;
spl_autoload_register(varray[$foo, 'nonstaticMethod']);
$funcs = spl_autoload_functions();
var_dump($funcs);
}
