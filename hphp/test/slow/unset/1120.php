<?hh

class A {
  public function foo() {
    unset($this);
    var_dump($this);
  }
}
function goo() {
  unset($this);
  var_dump($this);
}
<<__EntryPoint>> function main(): void {
$obj = new A;
$obj->foo();
goo();
unset($this);
var_dump($this);
}
