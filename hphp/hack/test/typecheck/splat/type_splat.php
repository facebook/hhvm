<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function foo(int $i, string $s): void {
  echo($i);
  echo($s);
}

function apply_tuple_mono(
  (function(int, string): void) $f,
  (int, string) $params,
): void {
  ($f)(...$params);
}

function apply_params_mono(
  (function(int, string): void) $f,
  int $i,
  string $s,
): void {
  ($f)($i, $s);
}

function test_mono(): void {
  apply_tuple_mono(foo<>, tuple(2, "A"));
  apply_params_mono(foo<>, 2, "A");
}

// This actually expects a tuple
function apply_tuple_poly<Targs as (mixed...)>(
  (function(...Targs): void) $f,
  Targs $params,
): void {
  // Check pretty-printing
  hh_show($f);
  hh_show($params);
  //($f)(...$params);
}

// This expects multiple params, but packed as a vec/tuple a la variadics
// As far as the runtime is concerned, it's variadic
function apply_params_poly<Targs as (mixed...)>(
  (function(...Targs): void) $f,
  ... Targs $params,
): void {
  hh_show($f);
  hh_show($params);
  //($f)(...$params);
}

<<__EntryPoint>>
function test_poly(): void {
  apply_tuple_poly(foo<>, tuple(2, "A"));
  apply_params_poly(foo<>, 2, "A");
}
