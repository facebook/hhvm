<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineVariadicTempHygiene {
  private function inline_me(int ...$rest, named int...): int {
    return $rest[0];
  }

  public function caller(): int {
    $__inline_method_arg_0 = 7;
    $__inline_method_named_arg_1 = 3;
    return $this->/*range-start*/inline_me/*range-end*/(
      $__inline_method_arg_0,
      surplus=$__inline_method_named_arg_1,
    );
  }
}
//
