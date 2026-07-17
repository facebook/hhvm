<?hh

class A {}
class C extends A {
  public function f(
    string $s1 = nameof C,
    string $t1 = C::class,
    string $s2 = nameof self,
    string $t2 = self::class,
  ): void {}
}

<<__EntryPoint>>
function main(): void {
  $c = new ReflectionClass("C");
  $m = $c->getMethod("f");
  foreach ($m->getParameters() as $p) {
    echo $p->getName();
    echo " => ";
    echo $p->getDefaultValueText();
    echo " = ";
    var_dump($p->getDefaultValue());
  }
}
