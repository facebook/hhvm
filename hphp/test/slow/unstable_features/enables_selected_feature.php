<?hh

// Test that only the selected feature is enabled
// If any of the unstable features becomes stable or are removed, replace
// with another unstable feature in this test

<<file:__EnableUnstableFeatures('case_types')>>

// No error here for using case type
case type t = bool;

interface I2 {
  // Error here due to use of union types
  public function error_here(): (I2 | bool);
}
