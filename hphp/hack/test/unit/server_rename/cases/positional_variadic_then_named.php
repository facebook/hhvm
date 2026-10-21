<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>
function original(string ...$rest, named int $b): int {
  return $b;
}
