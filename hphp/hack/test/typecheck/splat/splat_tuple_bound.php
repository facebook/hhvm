<?hh



interface I {
  public function foo<T as (mixed...), TExtraParam as (int, ...T)>(
    TExtraParam $x,
  ): void;
}
