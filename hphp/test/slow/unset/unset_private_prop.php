<?hh

class F {
  private $foo;
}
<<__EntryPoint>> function main() {
$f = new F();

unset($f->foo);
}
