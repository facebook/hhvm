<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

class Foo<T as (mixed...)> {
  <<__Memoize>>
  public function someMethod(...T $_): void {}
}

<<__Memoize>>
function some_function<T as (mixed...)>(...T $_): void {}
