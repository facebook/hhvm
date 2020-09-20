<?hh

// If there is no attribute then there should be a fatal
// when the feature is used

interface I1 where this as I1 {}

interface I2 {
  public function error_here(): (I2 | bool);
}
