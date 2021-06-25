<?hh

interface I {}
class C {}
trait T {}

function disj<T1, T2, T3>() : void where T3 super (T2 & T1) {}
function overlap<T1, T2, T3>() : void where T3 as (T2 & T1) {}

function tests() : void {
  disj<int, int, nothing>();    // type error (make sure test is working)
  overlap<int, string, int>();    // type error (make sure test is working)

  // primitive types are disjoint from classes, shapes, etc.
  disj<int, I, nothing>();
  disj<int, C, nothing>();
  disj<int, T, nothing>();
  disj<int, shape(), nothing>();
  disj<int, (function () : void), nothing>();
  disj<int, shape(), nothing>();
  disj<I, int, nothing>();
  disj<C, int, nothing>();
  disj<T, int, nothing>();
  disj<shape(), int, nothing>();
  disj<(function () : void), int, nothing>();
  disj<shape(), int, nothing>();

  // disjoint primitive tyeps
  disj<int, string, nothing>();
  disj<int, float, nothing>();
  disj<int, null, nothing>();
  disj<int, bool, nothing>();

  disj<string, int, nothing>();
  disj<string, float, nothing>();
  disj<string, null, nothing>();
  disj<string, bool, nothing>();
  disj<string, num, nothing>();

  disj<float, int, nothing>();
  disj<float, string, nothing>();
  disj<float, null, nothing>();
  disj<float, bool, nothing>();
  disj<float, arraykey, nothing>();

  disj<bool, int, nothing>();
  disj<bool, float, nothing>();
  disj<bool, null, nothing>();
  disj<bool, arraykey, nothing>();
  disj<bool, string, nothing>();
  disj<bool, num, nothing>();

  disj<null, int, nothing>();
  disj<null, float, nothing>();
  disj<null, bool, nothing>();
  disj<null, arraykey, nothing>();
  disj<null, string, nothing>();
  disj<null, num, nothing>();

  disj<arraykey, float, nothing>();
  disj<arraykey, null, nothing>();
  disj<arraykey, bool, nothing>();

  disj<num, string, nothing>();
  disj<num, null, nothing>();
  disj<num, bool, nothing>();

  overlap<int, int, int>();
  overlap<string, string, string>();
  overlap<float, float, float>();
  overlap<bool, bool, bool>();
  overlap<null, null, null>();
  overlap<arraykey, arraykey, arraykey>();
  overlap<num, num, num>();

  overlap<int, arraykey, int>();
  overlap<arraykey, int, int>();
  overlap<string, arraykey, string>();
  overlap<arraykey, string, string>();
  overlap<int, num, int>();
  overlap<num, int, int>();
  overlap<float, num, float>();
  overlap<num, float, float>();
  overlap<num, arraykey, int>();
  overlap<arraykey, num, int>();

  overlap<int, nonnull, int>();
  overlap<nonnull, int, int>();

}
