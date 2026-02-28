<?hh

class A {
  public function __construct(public int $i) {}
  public function incr(): void { $this->i++; }
}

class C {
  public function __construct(<<__Const>> public A $a)[] {}
}

<<__EntryPoint>>
function test(): void{
  $a = new A(1);
  $c = new C($a);
  var_dump($c);
  $a->incr();
  var_dump($c);
  $c->a->incr();
  var_dump($c);
}
