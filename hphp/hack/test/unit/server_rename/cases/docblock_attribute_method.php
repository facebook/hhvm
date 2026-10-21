<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>
class C {
    /** Keep the docblock and attribute on the renamed target. */
    <<__Memoize>>
    public function original(named int $b): int {
      return $b;
    }
}
