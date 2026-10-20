<?hh
//

class InlineContextDefaultSupplied {
  private function inline_me(int $line = __LINE__): int {
    return $line;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(42);
  }
}
//
