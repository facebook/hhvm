<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type B = string | vec<mixed>;
case type A<T as B> = T | int;

function f(vec<string> $s): void {
  hh_expect<A<vec<A<string>>>>($s);
}
