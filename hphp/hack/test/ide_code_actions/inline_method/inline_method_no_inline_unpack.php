<?hh
//

class InlineNoUnpack {
  private function inline_me(int ...$numbers): int {
    return count($numbers);
  }

  public function caller(): int {
    $values = vec[1, 2];
    return $this->/*range-start*/inline_me/*range-end*/(...$values);
  }
}
//
