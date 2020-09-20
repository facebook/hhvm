<?hh

class F {
  private $foo;
}
<<__EntryPoint>> function main(): void {
$f = new F();

unset($f->foo);
}
