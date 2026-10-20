<?hh

class InlineVariadicEmpty {
  private function inline_me(int ...$rest): int {
    return count($rest);
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/();
  }
}
//
