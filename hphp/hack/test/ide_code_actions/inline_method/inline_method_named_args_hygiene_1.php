<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineNamedArgsHygiene {
  private function inline_me(named string $x, named string $y): string {
    return $x.$y;
  }

  public function caller(): string {
    $x = "previous";
    return $this->/*range-start*/inline_me/*range-end*/(y="!", x="replacement").$x;
  }
}
//
