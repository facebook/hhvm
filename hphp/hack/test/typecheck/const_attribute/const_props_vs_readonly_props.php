<?hh

class A {
  public function __construct(public int $a) {}
  public function incr(): void {
    $this->a++;
  }
}

class Foo {
  public function __construct(public string $s) {}
}

class R {
  public static readonly ?Foo $static_foo = null;
  public function __construct(public readonly A $a, public readonly int $i) {}
}

class C {
  public static ?Foo $static_foo = null;
  public function __construct(<<__Const>> public A $a, <<__Const>> public int $i) {}
}

function test_readonly(): void {
  $a = new A(1);
  $r = new R($a, 2);
  R::$static_foo = new Foo("hello");
  R::$static_foo->s = "goodbye"; // fails: cannot modify props on readonly value
  $r->a->incr(); // fails: property returns readonly value, need to wrap in a readonly expression
  $a = readonly $r->a;
  $a->incr(); // readonly expression can only call readonly methods
  $r->i = 5;
}

function test_const(): void {
  $a = new A(1);
  $c = new C($a, 2);
  C::$static_foo = new Foo("hello"); // cannot make static props __Const
  C::$static_foo->s = "goodbye";
  $c->a->incr(); // valid
  $a = $c->a;
  $a->incr(); //valid
  $c->i = 5;  // invalid, const property cannot be mutated
}

<<__EntryPoint>>
function main(): void {
  test_readonly();
  test_const();
}
