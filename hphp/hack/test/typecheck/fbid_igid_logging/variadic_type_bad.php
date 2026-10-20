<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function named_sink(named int...): void {}
function positional(int ...$rest): void {}
function test_wrong_type(): void {
  named_sink(a = 'not an int', b = 'also not an int');
  positional('not an int', 'also not an int');
  named_sink(a = 'not an int', b = make_igid());
}
