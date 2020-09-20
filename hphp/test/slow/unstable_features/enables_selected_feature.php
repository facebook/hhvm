<?hh

// Test that only the selected feature is enabled
// If any of the unstable features becomes stable or are removed, replace
// with another unstable feature in this test

<<file:__EnableUnstableFeatures('class_level_where')>>

// No error here for using class level where
interface I1 where this as I1 {}

interface I2 {
  // Error here due to use of union types
  public function error_here(): (I2 | bool);
}
