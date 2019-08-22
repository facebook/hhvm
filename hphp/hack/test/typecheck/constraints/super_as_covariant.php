<?hh // strict

class Cov<+T> {}

/* HH_FIXME[4336] */
function UseAs<Tv, Tu as Tv>(Cov<Tv> $x, Cov<Tu> $y): Tu {
}

/* HH_FIXME[4336] */
function UseSuper<Tv super Tu, Tu>(Cov<Tv> $x, Cov<Tu> $y): Tu {
}

class MyBase {}
class MyDerived extends MyBase {}

function DoesWork(Cov<MyBase> $cb, Cov<MyDerived> $cd): MyDerived {
  return UseSuper($cb, $cd);
}

function DoesNotWork(Cov<MyBase> $cb, Cov<MyDerived> $cd): MyDerived {
  return UseAs($cb, $cd);
}
