<?hh

class InlineDefaultCollection {
  private function inline_me(vec<int> $values = vec[1, 2]): int {
    return count($values);
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/();
  }
}
//
