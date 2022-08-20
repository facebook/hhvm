<?hh

class Foo2 {
  public mixed $x;
  public function __construct(num $x = 0) {
    $this->x = $x;
  }
}

class Foo1 {
  public vec<Foo2> $f2;
  public readonly vec<Foo2> $ro_f2;
}

<<__EntryPoint>>
function test(): void {
  $t = new Foo1();
  $t1 = new Foo1();
  $t->f2 = vec[new Foo2()];
  unset($t->f2[0]->x);
  $t1->ro_f2 = vec[new Foo2()];
  unset($t1->ro_f2[0]->x);
}
