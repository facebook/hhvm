<?hh

class InlineGenericDefaultSupplied {
  private function inline_me<T>(vec<T> $values = vec<T>[]): vec<T> {
    return $values;
  }

  public function caller(): vec<int> {
    return $this->/*range-start*/inline_me/*range-end*/<int>(vec[1]);
  }
}
