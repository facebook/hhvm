<?hh

class C {
  const type T1 = int;
  const type T2 = shape(
    'a' => self::T1,
    'b' => this::T1,
  );
}

class D {
  const type T1 = string;
  public function f(mixed $x) :mixed{
    $e = new E();
    $e->f<C::T2>($x);
  }
}

class E {
  const type T1 = bool;
  public function f<reify T>(mixed $x) :mixed{
    var_dump($x is T);
  }
}
<<__EntryPoint>> function main(): void {
$d = new D();

$d->f(shape('a' => 1, 'b' => 2));
$d->f(shape('a' => 'string', 'b' => 2));
$d->f(shape('a' => 1, 'b' => 'string'));
$d->f(shape('a' => 'string', 'b' => 'string'));
}
