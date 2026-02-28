<?hh

class A {}
class C extends A {
  public function f(
    string $s1 = nameof C,
    string $t1 = C::class,
    string $s2 = nameof self,
    string $t2 = self::class,
    string $s4 = nameof parent,
    string $t4 = parent::class,
    string $s3 = nameof static,
    // TODO: this case is buggy but unused
    string $t3 = static::class,
  ): void {}
}

<<__EntryPoint>>
function main(): void {
  $c = new ReflectionClass("C");
  $m = $c->getMethod("f");
  foreach($m->getParameters() as $p) {
    echo $p->getName();
    echo " => ";
    echo $p->getDefaultValueText();
    echo " = ";
    var_dump($p->getDefaultValue());
  }
}
