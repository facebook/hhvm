<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlinePositionalNamedReadOrder {
  private function inline_me(int $p, named int $a): int {
    return $p * 10 + $a;
  }

  private function bump(inout int $x): int {
    $x = $x + 1;
    return $x;
  }

  public function caller(): int {
    $x = 1;
    return $this->/*range-start*/inline_me/*range-end*/($x, a=$this->bump(inout $x));
  }
}
