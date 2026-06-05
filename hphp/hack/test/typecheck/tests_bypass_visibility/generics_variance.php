<?hh

class WWWTest {}

// Covariant: T can appear in return position but not parameter position
class CovMethod<+T> {
  // Normal private: relaxed, covariant T allowed in contravariant position (no error)
  private function normal_priv(T $x): void {}

  // With attribute: enforced, covariant T in contravariant position is an error
  <<__TestsBypassVisibility>>
  private function bypass_priv(T $x): void {}

  // Protected: always enforced (error)
  protected function prot(T $x): void {}
}

// Contravariant: T can appear in parameter position but not return position
class ContraMethod<-T> {
  // Normal private: relaxed, contravariant T allowed in covariant position (no error)
  private function normal_priv(): T {
    throw new \Exception("");
  }

  // With attribute: enforced, contravariant T in covariant position is an error
  <<__TestsBypassVisibility>>
  private function bypass_priv(): T {
    throw new \Exception("");
  }
}

// Property variance
class CovProp<+T> {
  // Normal private: relaxed (no error)
  private ?T $normal_prop = null;

  // With attribute: enforced, covariant T in invariant position is an error
  <<__TestsBypassVisibility>>
  private ?T $bypass_prop = null;

  // Protected: always enforced (error)
  protected ?T $prot_prop = null;
}
