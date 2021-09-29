<?hh

<<file: __EnableUnstableFeatures('readonly')>>

<<__EntryPoint>>
function test(): void {
  $loc = readonly Map {1 => Map {1 => 2}};
  try {
    $loc[1][1] = 5; // loc must be COW.
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  $loc = readonly vec[Map {1 => 2}];
  $loc[0] = Map{1 => 3}; // ok
  $loc[0][1] = 5; // [0] must be COW
}
