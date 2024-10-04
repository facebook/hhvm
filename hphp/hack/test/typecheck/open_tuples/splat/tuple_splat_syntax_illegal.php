<?hh

<<file: __EnableUnstableFeatures('open_tuples')>>

// Legal tuple types
type T1 = (bool, ...(int, string));
interface I {
  public function make<T as (mixed...)>(int $arg): (int, ...T);
}
type T2<T> = (float, ...T);

// These types will be expanded in decling
function test1(T1 $x1, T2<(int, string)> $x2): void {
}
// These types will be parsed directly
function other<T as (mixed...)>(
  (bool, ...(int, string)) $x1,
  (float, ...T) $x2,
): void {
}
