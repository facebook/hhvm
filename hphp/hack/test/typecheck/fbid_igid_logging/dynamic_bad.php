<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function sink_positional(int $a, int $b): void {}
function sink(named int $other, named int $instagram_id): void {}

function test_dynamic(dynamic $d): void {
  sink_positional($d, make_igid());
  sink(instagram_id = make_igid(), other = $d);
}
