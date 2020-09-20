<?hh // strict

function foo(int $x): int {return $x + 2;}

interface I {
  public function bar(): void;
}

abstract class B implements I {
  public abstract static function genBaz(): Awaitable<void>;
}

class C extends B {
  final public function bar(): void {}

  public static async function genBaz(): Awaitable<void> {}
}

async function test(): Awaitable<void> {
  $foo = 'foo';
  $bar = 'bar';
  $c = 'C';
  $foo_dynamic = HH\dynamic_fun($foo);
  hh_show($foo_dynamic);
  $gen_baz_dynamic = HH\dynamic_class_meth($c, 'genBaz');
  hh_show($gen_baz_dynamic);
  $foo_called = $foo_dynamic();
  hh_show($foo_called);
}
