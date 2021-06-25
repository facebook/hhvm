<?hh

function disj<T1, T2, T3>() : void where T3 super (T2 & T1) {}
function overlap<T1, T2, T3>() : void where T3 as (T2 & T1) {}

interface I {}
class C {}
trait T {}

enum E1 : int {
  C1 = 1;
}

enum E2 : int {
  C2 = 2;
}

enum E3 : int as int {
  C1 = 1;
}

enum E4 : string as string {
  C1 = "1";
}

enum E5 : string as string {
  C2 = "2";
}

enum E6 : string {
  C1 = "1";
}



function test() : void {
  disj<E1, E2, nothing>(); // type error. We don't consider these disjoint, even though they actually are.
  // This could possibly be improved, but might be expensive to check.

  disj<E1, E5, nothing>(); // type error. opaque
  disj<E1, string, nothing>(); // type error. opaque
  disj<int, E1, nothing>(); // type error. opaque
  disj<E4, E5, nothing>(); // type error. We don't consider these disjoint, even though they actually are.
  // This could possibly be improved, but might be expensive to check.

  disj<arraykey, E1, nothing>(); // type error. opaque

  disj<E3, string, nothing>();
  disj<E3, E4, nothing>();
  disj<int, E4, nothing>();
  disj<E4, E3, nothing>();
  overlap<arraykey, E4, E4>();
  overlap<E4, string, E4>();
  overlap<arraykey, E3, E3>();
  overlap<E3, int, E3>();
  overlap<num, E3, E3>();
  disj<E5, num, nothing>();

  disj<E1, I, nothing>();
  disj<C, E1, nothing>();
  disj<E1, T, nothing>();
}
