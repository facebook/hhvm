<?hh

<<file: __EnableUnstableFeatures('readonly')>>

<<__EntryPoint>>
function test(): void {
  $loc = readonly Map{1 => Map{1 => 2}};
  $loc[1][1] = 5; // loc must be COW. [1] must be COW.
  $loc = readonly vec[Map{1 => 2}];
  $loc[0][1] = 5; // [0] must be COW
  $loc[0] = Map{1 => 3}; // ok
}
