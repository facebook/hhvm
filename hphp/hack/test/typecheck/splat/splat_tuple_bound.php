<?hh

<<file: __EnableUnstableFeatures('type_splat', 'open_tuples')>>

interface I {
  public function foo<T as (mixed...), TExtraParam as (int, ...T)>(
    TExtraParam $x,
  ): void;
}
