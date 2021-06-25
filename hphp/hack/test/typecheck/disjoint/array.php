<?hh

function disj<T1, T2, T3>() : void where T3 super (T2 & T1) {}
function overlap<T1, T2, T3>() : void where T3 as (T2 & T1) {}

class C {}

type t1 = shape('a' => int);
type t2 = shape('b' => int);
type t3 = shape('a' => int, ...);
type t4 = shape('b' => int, ...);

function test() : void {
  disj<(int), (int, int), nothing>();
  disj<(int, string), (int, int), nothing>();
  overlap<(int, arraykey), (num, string), (int, string)>();

  disj<(int, arraykey), vec<mixed>, nothing>(); // type error
  disj<(int, arraykey), C, nothing>();

  overlap<t1, t1, t1>();
  disj<t1, dict<arraykey, mixed>, nothing>(); // type error


  disj<t1, t2, nothing>();
  disj<t3, t2, nothing>();
  disj<t3, t4, nothing>(); // type error
  disj<t2, t4, nothing>(); // type error
}
