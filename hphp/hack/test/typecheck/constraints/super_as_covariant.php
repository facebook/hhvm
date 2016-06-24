<?hh // strict

class Cov<+T> {}

function UseAs<Tv, Tu as Tv>(Cov<Tv> $x, Cov<Tu> $y): Tu {
  //UNSAFE
}

function UseSuper<Tv super Tu, Tu>(Cov<Tv> $x, Cov<Tu> $y): Tu {
  //UNSAFE
}

class MyBase {}
class MyDerived extends MyBase {}

function DoesWork(Cov<MyBase> $cb, Cov<MyDerived> $cd): MyDerived {
  return UseSuper($cb, $cd);
}

function DoesNotWork(Cov<MyBase> $cb, Cov<MyDerived> $cd): MyDerived {
  return UseAs($cb, $cd);
}
