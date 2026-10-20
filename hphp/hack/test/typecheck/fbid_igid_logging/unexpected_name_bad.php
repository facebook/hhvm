<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function sink(named int $instagram_id): void {}
function test_unexpected_name(): void {
  sink(other = make_igid());
}
