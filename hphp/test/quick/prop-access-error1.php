<?hh

class B {
  protected $p;
}

class C extends B {}

<<__EntryPoint>> function main(): void {
  $o = new C;
  var_dump($o->p);
}
