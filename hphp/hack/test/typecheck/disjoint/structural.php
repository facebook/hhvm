<?hh

function disj<T1, T2, T3>() : void where T3 super (T2 & T1) {}
function overlap<T1, T2, T3>() : void where T3 as (T2 & T1) {}

class C {}
interface I {}
class C2 extends C implements I {}
final class FC {}

function test() : void {

  disj<?int, string, nothing>();
  disj<float, ?C, nothing>();
  overlap<?int, int, int>();
  overlap<null, ?int, null>();
  overlap<nonnull, ?int, int>();

  disj<float, (int | string), nothing>();
  overlap<(int | string), int, int>();

  disj<(C & I), FC, nothing>();
  overlap<C2, (C & I), C2>();
}
