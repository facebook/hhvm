<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>
class C {
  public function original(named int $b, string ...$rest): int {
    return $b;
  }
}
