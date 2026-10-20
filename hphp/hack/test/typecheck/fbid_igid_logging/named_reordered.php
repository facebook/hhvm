<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function sink(named int $other, named int $instagram_id): void {}

function sink_variadic(named int...): void {}

function sink_positional(int $a, int $b): void {}

function test_reordered(): void {
  sink(instagram_id = make_igid(), other = 456);
}

function test_ordered(): void {
  sink(other = 456, instagram_id = make_igid());
}

function test_variadic(): void {
  sink_variadic(first = make_igid(), second = make_igid());
}

function test_positional(): void {
  sink_positional(make_igid(), 456);
}
//
