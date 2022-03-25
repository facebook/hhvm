<?hh

class Contra<-T> {}
class In<T> {}

interface I {
  abstract const type Ti as num;
}

class A {          // covariant pos.    vvv  (double flip)
  public function numInCovar_Sub(Contra<num> $t): void {}
  public function numInCovar_Eq(Contra<num> $t): void {}
  public function numInCovar_Super(Contra<num> $_): void {}
  // see: https://wuciawe.github.io/scala/2014/09/04/variance-in-scala.html#checking-variance-annotation

  public function numInContravar_Super(num $_): void {}
  public function numInContravar_Sub(num $t): void {}
  public function numInContravar_Eq(num $t): void {}
  //             contravariant pos. ^^^ (single flip)
  public function numInInvar_Super(In<num> $_): void {}
  public function numInInvar_Sub(In<num> $t): void {}
  public function numInInvar_Eq(In<num> $t): void {}
} //                invariant pos. ^^^ (single flip then override)

class B<T as I> extends A {
  <<__Override>>
  public function numInCovar_Sub<TTi>(Contra<TTi> $_): void where TTi as T::Ti {
  }
  // iff Contra<num> as Contra<TTi> for some TTi such that TTi <: T::Ti
  // iff TTi <: num
  // Let TTi = T::Ti and we're done.

  <<__Override>> // CURRENT desugaring of polymorphic coeffects -> this must type-check but it doesn't!!!
  public function numInCovar_Eq<TTi>(Contra<TTi> $_): void where TTi = T::Ti {} // OK
  // iff Contra<num> as Contra<TTi> for some TTi such that TTi=T::Ti
  // iff Contra<num> as Contra<T::Ti> (by equality)
  // iff T::Ti as num
  // therefore it should be OK, but Hack errs: expected `num` but got `T::Ti`

  <<__Override>>
  public function numInCovar_Super<TTi>(Contra<TTi> $_): void where TTi super T::Ti {} // ERROR
  // iff Contra<num> as Contra<TTi> for some TTi such that T::Ti <: TTi
  // iff TTi <: num
  // Just let TTi = num and we're done.

  <<__Override>>
  public function numInContravar_Super<TTi>(TTi $_): void where TTi super T::Ti {} // OK
  // iff num <: TTi for some TTi such that T::Ti <: TTi
  // Just set TTi = num, and we're done.

  <<__Override>>
  public function numInContravar_Sub<TTi>(TTi $_): void where TTi as T::Ti {} // ERROR
  // iff num <: TTi for some TTi such that TTi <: T::Ti
  // So we have num <: TTi <: T::Ti which has no solution

  <<__Override>>
  public function numInContravar_Eq<TTi>(TTi $_): void where TTi = T::Ti {} // ERROR
  // iff num <: TTi for some TTi such that TTi = T::Ti
  // There is no solution

  <<__Override>>
  public function numInInvar_Super<TTi>(In<TTi> $_): void where TTi as T::Ti {} // wrongly OK (was ERROR before D34600464)
  // Need In<num> <: In<TTi> such that TTi <: T::Ti
  // No solution

  <<__Override>>
  public function numInInvar_Sub<TTi>(In<TTi> $_): void where TTi super T::Ti {} // OK
  // Need In<num> <: In<TTi> such that T::Ti <: TTi
  // Just set TTi = num and we're done.

  <<__Override>>
  public function numInInvar_Eq<TTi>(In<TTi> $_): void where TTi = T::Ti {} // ERROR
  // Need In<num> <: In<TTi> such that TTi = T::Ti
  // No solution

}
