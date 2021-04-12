<?hh

class C {
  public $m = Map {'a' => 1};
}

<<__EntryPoint>> function main(): void {
  $c = new C;
  $c->m['b'] = 2;
  var_dump($c);
  gc_collect_cycles();
  var_dump($c);
}
