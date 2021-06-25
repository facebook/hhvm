<?hh

function disj<T1, T2, T3>() : void where T3 super (T2 & T1) {}
function overlap<T1, T2, T3>() : void where T3 as (T2 & T1) {}

class C0 implements I1 {}
class C1 extends C0 { use T1; }
class C2 implements I1 {}
final class FC extends C1 implements I1 {}
interface I1 {}
interface I2 {}
trait T1 { require implements I1; }
trait T2 {}

function tests() : void {
  disj<FC, I2, nothing>();
  disj<I2, FC, nothing>();
  disj<C1, C2, nothing>();

  overlap<C1, C1, C1>();
  overlap<I1, I1, I1>();
  disj<C1, I2, nothing>(); // type error, they are not disjoint
  disj<I2, C1, nothing>(); // type error, they are not disjoint
  overlap<C1, I1, C1>();
  overlap<I1, C1, C1>();
  overlap<FC, I1, FC>();
  overlap<I1, FC, FC>();
  overlap<C0, FC, FC>();
  overlap<FC, C0, FC>();
  disj<I1, I2, nothing>(); // type error, they are not disjoint
  disj<I2, I1, nothing>(); // type error, they are not disjoint

  disj<FC, T2, nothing>();
  disj<T2, FC, nothing>();

  disj<T1, I2, nothing>(); // type error, they are not disjoint
  disj<I2, T1, nothing>(); // type error, they are not disjoint
  disj<T1, T2, nothing>(); // type error, they are not disjoint
  disj<T2, T1, nothing>(); // type error, they are not disjoint
  disj<C2, T2, nothing>(); // type error, they are not disjoint
  disj<T2, C2, nothing>(); // type error, they are not disjoint

  overlap<T2, T2, T2>();
  overlap<FC, T1, FC>();
  overlap<T1, FC, FC>();
  overlap<T1, I1, T1>();
  overlap<I1, T1, T1>();
  overlap<T1, C1, C1>();
  overlap<C1, T1, C1>();
}
