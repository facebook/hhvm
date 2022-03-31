<?hh

interface PredBase<-T , Tcontext > {}

interface Pred<-T , Tcontext >
  extends PredBase<T, Tcontext> {
}

interface PredRange<Tv , Tcontext >
  extends Pred<Tv, Tcontext> {
}

class C<T1 , T2 > implements PredRange<T1, T2> {}

function and_<Tv, Tcontext >(
    Pred<Tv, Tcontext> $predicates,
    Pred<Tv, Tcontext> $predicates2,
  ): Pred<Tv, Tcontext> {
  return new C();
}

function greaterThanOrEquals<Tv , Tcontext >(
    Tv $value,
  ): PredRange<Tv, Tcontext> {
  return new C();
}

function lessThan< Tv as dynamic , Tcontext >(
    Tv $value,
  ): PredRange<Tv, Tcontext> {
  return new C();
}

  function f(
    int $lower_time,
    int $upper_time,
  ): void
   {
    $lt =lessThan($upper_time);
    $gte =greaterThanOrEquals($lower_time);
    $and = and_($lt, $gte);
  }
