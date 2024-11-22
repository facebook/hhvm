<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function expectSplatTuple<T as (string, arraykey...)>((int, ...T) $tup): void {
}

function test1(): void {
  // Too few
  expectSplatTuple(tuple());
  // Too few
  expectSplatTuple(tuple(3));
  // Wrong types
  expectSplatTuple(tuple(2, 4.5));
  // Wrong types
  expectSplatTuple(tuple(2, "A", false));
}
