<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineNamedArgsReadOrder {
  private function inline_me(named int $a, named int $b): int {
    return $b;
  }

  private function bump(inout int $x): int {
    $x = $x + 1;
    return $x;
  }

  public function caller(): int {
    $x = 1;
    return $this->/*range-start*/inline_me/*range-end*/(b=$x, a=$this->bump(inout $x));
  }
}
//
