<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class MyList<T> {
  public function __construct(public T $item) { }
}
final class Contra<-T> { }
final class Cov<+TCov> {
  public function onlycontravariantsuper<T super TCov>(T $_x):void { }
}
interface I { }
final class C<+Tc> {
  public function nowhere<T>(int $_x):void { }
  public function onlycovariant1<T>(): T {
    throw new Exception("a");
  }
  public function onlycontravariant1<T as I>(T $_x): void { }
  public function onlycontravariant2<T>(): Contra<T> {
    return new Contra();
  }
  public function onlycontravariant3<T>(T $_x): Contra<T> {
    return new Contra();
  }
  public function onlycovariant2<T>((function(T):void) $_f) : void { }
}

interface IVCB { }
interface IEMVB<-TVC as IVCB> { }
interface IPRB<-TVC, -T> { }
  function blah<
    TVC as IVCB,
    TEMV as IEMVB<TVC>
  >(
    vec<IPRB<TVC, TEMV>> $_rules,
  ):void { }

function mysort<Tv, T as Container<Tv>>(
  inout T $_arg,
): bool {
  return false;
}

function mysort_where<Tv,T>(inout T $_arg): bool where T as Container<Tv> { return false; }
