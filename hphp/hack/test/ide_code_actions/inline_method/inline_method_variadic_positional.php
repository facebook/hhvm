<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineVariadicPositional {
  private function inline_me(int $head, int ...$rest, named int $scale = 1): int {
    return $head + $rest[0] * $scale;
  }

  private function observe(int $x): int {
    return $x;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(
      $this->observe(1),
      scale=$this->observe(2),
      $this->observe(3),
      $this->observe(4),
    );
  }
}
//
