<?hh

class A {
  public function Gen() :AsyncGenerator<mixed,mixed,void>{
    var_dump($this);
    yield 1; yield 2; yield 3;
  }

  public static function SGen() :AsyncGenerator<mixed,mixed,void>{
    var_dump(static::class);
    yield 4; yield 5; yield 6;
  }
}

<<__EntryPoint>> function main(): void {
$a = new A();
foreach ($a->Gen() as $num) { var_dump($num); }
foreach (A::SGen() as $num) { var_dump($num); }
}
