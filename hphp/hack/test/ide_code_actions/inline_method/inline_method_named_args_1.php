<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineNamedArgs {
  private function inline_me(named string $param_a, named int $param_b): string {
    $z = $param_a;
    if ($param_b > 1) {
      $z = $param_a.$param_b;
    }
    return $z;
  }
  public function caller(): string {
    return $this->/*range-start*/inline_me/*range-end*/(param_b=2, param_a="hello");
  }
}
//
