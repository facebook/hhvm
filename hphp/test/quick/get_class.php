<?hh

class A {
  public function f($x) :mixed{
    var_dump(HH\dynamic_fun($x)($this));
    var_dump(get_class($this));
  }
}
<<__EntryPoint>> function main(): void {
$a = new A();
$a->f('get_class');
var_dump(get_class($a));
var_dump(get_class(null));
}
