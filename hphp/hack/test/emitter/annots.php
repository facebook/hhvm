<?hh // strict

class A {
  const A = 5;
}

class Nus {
  public function __construct(public int $baz) {}
  public function lol(this $x): this { return $this; }

  public function bar<T>(T $a, Nus $b, mixed $c, resource $d, string $e,
                         float $f, num $g, Awaitable<int> $h): void {}

  public function shp(shape('a' => int, 'b' => string) $x): void {}
  public function shp2(shape(A::A => int) $x): void {}
  public function arr(array<string, int> $x, array<string> $y): void {}
  /* HH_FIXME[4045] */
  public function arr2(array $x): void {}
  public function noret(): noreturn { while (true); }

  public function tup((int, int) $x): (int, int) { return $x; }

  public function func(): ?(function (string): int) { return null; }
}

function foo(Nus $a, ?Nus $b, arraykey $c, ?int $d): void {}

/* HH_FIXME[4032]: hh_single_type_check :( */
function dump_func_obj($x): void {
  echo $x->getName(), ": ", $x->getReturnTypeText(), "\n";
  foreach ($x->getParameters() as $p) {
    echo "  ", $p->getName(), ": ", $p->getTypehintText(), " -- ",
      $p->getTypeText(), "\n";
  }
}
function dump_func(string $name): void {
  /* HH_FIXME[2049] */
  dump_func_obj(new ReflectionFunction($name));
}
function dump_class(string $name): void {
  /* HH_FIXME[2049] */
  $cls = new ReflectionClass($name);
  $a = array();
  foreach ($cls->getMethods() as $meth) $a[$meth->getShortName()] = $meth;
  /* HH_FIXME[2049] */
  /* HH_FIXME[4107] */
  krsort($a);
  foreach ($a as $meth) dump_func_obj($meth);
}

function test(): void {
  dump_func('foo');
  dump_class('Nus');
}
