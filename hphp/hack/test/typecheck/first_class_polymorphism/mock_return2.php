<?hh

class Mock<Tfun> {

  private function __construct(private HH\FunctionRef<Tfun> $fn): void {}

  <<__NoAutoLikes>>
  final public static function mockFunction(HH\FunctionRef<Tfun> $fn): Mock<Tfun> {
    return new Mock($fn);
  }

  public function mockReturn<Tr>(Tr $v): this
  where
    Tfun super (readonly function(mixed...)[]: Tr) {
    return $this;
  }

}

class Pred<-T1, T2> {
  public function negate(): this {
    return $this;
  }
}

class Pr {
  public static function not<T1, T2>(Pred<T1, T2> $x): Pred<T1, T2> {
    return $x->negate();
  }
  public static function alwaysTrue<T1, T2>(): Pred<T1, T2> {
    return new Pred();
  }
}

function test(): void {
  Mock::mockFunction(Pr::not<>)->mockReturn(Pr::alwaysTrue());
}
