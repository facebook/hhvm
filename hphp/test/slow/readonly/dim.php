<?hh

class Foo2 {
  public int $x;
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
  $t->f2[0]->x = 10; // ok
  $t1->ro_f2 = vec[new Foo2()];
  try {
    $t1->ro_f2[0]->x = 10;   // error, dim is readonly
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  $t1->ro_f2 = vec[new Foo2(10, 20)];
  try {
    $y = $t1->ro_f2[0]->x;   // error, dim is readonly
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
