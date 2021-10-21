<?hh

// Reported in https://github.com/facebook/hhvm/issues/8867 but could not repro
function example((function(int, int): int) $pick = HH\Lib\SecureRandom\int<>): int {
  return $pick(0, 1000);
}
