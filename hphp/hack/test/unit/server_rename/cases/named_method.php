<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>
class C {
  public function original(named int $b, int $a): int {
    return $a + $b;
  }
}
