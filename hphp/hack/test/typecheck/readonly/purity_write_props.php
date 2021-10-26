<?hh
<<file: __EnableUnstableFeatures('readonly')>>

function foo()[]: void {
  $y = readonly dict<string, int>[];
  // should be able to write to the dict
  $y["2"] = 5;
}
