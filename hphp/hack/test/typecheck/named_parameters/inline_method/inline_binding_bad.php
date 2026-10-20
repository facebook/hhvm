<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineBindingBad {
  private function names(named string $param_a, named int $param_b): string {
    return $param_a.$param_b;
  }

  private function variadic(int ...$rest, named string...): int {
    return count($rest);
  }

  public function caller(): void {
    $a = $this->names(param_b='wrong', param_a=2);
    $b = $this->names(param_a='missing param_b');
    $c = $this->names(param_b=2, unknown='wrong name');
    $d = $this->variadic('wrong positional', extra=42);
  }
}
//
