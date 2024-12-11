<?hh
<<file: __EnableUnstableFeatures('function_references')>>

function mockFunction<Tfun>(HH\FunctionRef<Tfun> $_): void {}

function top_level_mock_me<T as arraykey>(T $x): void {}

function mock_it(): void {
  $f = top_level_mock_me<>;
  mockFunction($f);
}
